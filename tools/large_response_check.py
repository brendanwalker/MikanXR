#!/usr/bin/env python3
"""Check that a large binary response does not wedge a client's websocket connection.

Fetches a model stencil's render geometry over a raw websocket, then sends a small request on the
same connection and requires an answer. A server that stops reading after the large response fails
the second step while still streaming events, so the connection looks alive but answers nothing.

    build\\src\\Editor\\Release\\Mikan.exe
    python tools/large_response_check.py

The stencil is found through the automation server unless --stencil names one, so the loaded
project needs a model stencil with geometry (a depth proxy mesh capture is the usual one). The
check only exercises the interesting path when the response exceeds one websocket chunk, 32 KiB,
and says so when it does not.

This is a manual check against a running editor, not part of the test suites.
"""

import argparse
import base64
import json
import os
import socket
import struct
import sys
import time

WEBSOCKET_PORT = 8080
AUTOMATION_PORT = 21120
WEBSOCKET_PROTOCOL = "Mikan-0"
CHUNK_SIZE = 1 << 15

# The follow-up request is tiny, so it either answers at once or the connection is wedged. Waiting
# out the full geometry timeout for it would only make a failing check slow.
FOLLOW_UP_TIMEOUT = 10.0


def automation_command(host, port, command, timeout):
    """Send one line to the automation server and return its framed reply lines."""
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
    except OSError as error:
        raise SystemExit(f"error: could not reach the automation server on {host}:{port}: {error}")

    with sock:
        sock.settimeout(timeout)
        sock.sendall((command + "\n").encode())
        data = b""

        def read_line():
            nonlocal data
            while b"\n" not in data:
                chunk = sock.recv(4096)
                if not chunk:
                    raise SystemExit("error: automation server closed the connection")
                data += chunk
            line, _, rest = data.partition(b"\n")
            data = rest
            return line.decode(errors="replace").rstrip("\r")

        count = read_line()
        try:
            return [read_line() for _ in range(int(count))]
        except ValueError:
            raise SystemExit(f"error: expected a reply count from the automation server, got '{count}'")


def find_model_stencil(host, automation_port, timeout):
    """Return the first model stencil component id in the loaded project."""
    lines = automation_command(host, automation_port, "component list ModelStencilSystem", timeout)
    for line in lines:
        if line.startswith("error:"):
            raise SystemExit(f"error: {line}")
        component_id = line.split()[0]
        if component_id.lstrip("-").isdigit():
            return int(component_id)

    raise SystemExit("error: the loaded project has no model stencil to fetch geometry from")


def open_websocket(host, port, timeout):
    """Complete a websocket handshake and return the socket plus any buffered bytes."""
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
    except OSError as error:
        raise SystemExit(f"error: could not reach the Mikan websocket on {host}:{port}: {error}")

    key = base64.b64encode(os.urandom(16)).decode()
    sock.sendall((
        f"GET / HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        f"Upgrade: websocket\r\n"
        f"Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {key}\r\n"
        f"Sec-WebSocket-Version: 13\r\n"
        f"Sec-WebSocket-Protocol: {WEBSOCKET_PROTOCOL}\r\n\r\n").encode())

    response = b""
    while b"\r\n\r\n" not in response:
        chunk = sock.recv(4096)
        if not chunk:
            raise SystemExit("error: the server closed the connection during the handshake")
        response += chunk

    header, _, rest = response.partition(b"\r\n\r\n")
    if b" 101" not in header.split(b"\r\n")[0]:
        raise SystemExit("error: websocket handshake refused: " + header.split(b"\r\n")[0].decode(errors="replace"))

    return sock, rest


def send_request(sock, request):
    """Send one masked text frame holding the request JSON."""
    payload = json.dumps(request).encode()
    mask = os.urandom(4)
    length = len(payload)

    header = bytes([0x81])
    if length < 126:
        header += bytes([0x80 | length])
    elif length < 65536:
        header += bytes([0x80 | 126]) + struct.pack(">H", length)
    else:
        header += bytes([0x80 | 127]) + struct.pack(">Q", length)

    sock.sendall(header + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(payload)))


class MessageReader:
    """Reassembles websocket frames into whole messages, dropping nothing."""

    def __init__(self, sock, buffered):
        self.sock = sock
        self.buffer = buffered
        self.fragment_opcode = None
        self.fragment = b""

    def next_message(self, deadline):
        """Return (opcode, payload) for the next complete message, or None past the deadline."""
        while True:
            message = self._take_message()
            if message:
                return message

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return None

            self.sock.settimeout(remaining)
            try:
                chunk = self.sock.recv(1 << 20)
            except socket.timeout:
                return None
            if not chunk:
                raise SystemExit("error: the server closed the connection")
            self.buffer += chunk

    def _take_message(self):
        while len(self.buffer) >= 2:
            is_final = self.buffer[0] & 0x80
            opcode = self.buffer[0] & 0x0F
            length = self.buffer[1] & 0x7F
            offset = 2

            if length == 126:
                if len(self.buffer) < 4:
                    return None
                length, offset = struct.unpack(">H", self.buffer[2:4])[0], 4
            elif length == 127:
                if len(self.buffer) < 10:
                    return None
                length, offset = struct.unpack(">Q", self.buffer[2:10])[0], 10

            if len(self.buffer) < offset + length:
                return None

            payload = self.buffer[offset:offset + length]
            self.buffer = self.buffer[offset + length:]

            if opcode == 0:
                self.fragment += payload
                if is_final:
                    whole = (self.fragment_opcode, self.fragment)
                    self.fragment_opcode, self.fragment = None, b""
                    return whole
            elif is_final:
                return opcode, payload
            else:
                self.fragment_opcode, self.fragment = opcode, payload

        return None


def await_response(reader, request_id, timeout):
    """Return the response for request_id, skipping the editor's event stream."""
    deadline = time.monotonic() + timeout
    while True:
        message = reader.next_message(deadline)
        if message is None:
            return None

        opcode, payload = message
        if opcode == 2:
            # Binary responses carry no readable id; only one is ever in flight here.
            return payload
        if opcode != 1:
            continue

        try:
            body = json.loads(payload.decode("utf-8", errors="replace"))
        except ValueError:
            continue
        if body.get("requestId") == request_id:
            return payload


def main():
    parser = argparse.ArgumentParser(description="Check that a large binary response leaves the connection usable.")
    parser.add_argument("--host", default="127.0.0.1", help="editor host")
    parser.add_argument("--port", type=int, default=WEBSOCKET_PORT, help="client websocket port")
    parser.add_argument("--automation-port", type=int, default=AUTOMATION_PORT,
                        help="automation server port, used to find a model stencil")
    parser.add_argument("--stencil", type=int, help="model stencil component id, instead of discovering one")
    parser.add_argument("--rounds", type=int, default=3, help="how many times to fetch the geometry")
    parser.add_argument("--timeout", type=float, default=60.0, help="seconds to wait for each response")
    args = parser.parse_args()

    stencil_id = args.stencil
    if stencil_id is None:
        stencil_id = find_model_stencil(args.host, args.automation_port, args.timeout)
    print(f"model stencil {stencil_id}")

    sock, buffered = open_websocket(args.host, args.port, args.timeout)
    reader = MessageReader(sock, buffered)
    request_id = 0
    smallest = None

    with sock:
        for round_index in range(1, args.rounds + 1):
            request_id += 1
            started = time.monotonic()
            send_request(sock, {
                "requestTypeName": "GetModelStencilRenderGeometry",
                "requestId": request_id,
                "stencilId": stencil_id})
            geometry = await_response(reader, request_id, args.timeout)
            if geometry is None:
                raise SystemExit(f"error: round {round_index}: no geometry response within {args.timeout:g}s")

            elapsed = time.monotonic() - started
            smallest = len(geometry) if smallest is None else min(smallest, len(geometry))
            print(f"round {round_index}: geometry {len(geometry):,} bytes in {elapsed:.2f}s")

            # The point of the check: the same connection must still answer afterwards.
            request_id += 1
            send_request(sock, {"requestTypeName": "GetAppStageInfo", "requestId": request_id})
            if await_response(reader, request_id, min(args.timeout, FOLLOW_UP_TIMEOUT)) is None:
                raise SystemExit(
                    f"error: round {round_index}: the connection stopped answering after the large response")

            print(f"round {round_index}: connection still answers")

    if smallest is not None and smallest < CHUNK_SIZE:
        print(f"warning: the largest response was under {CHUNK_SIZE:,} bytes, so this ran without "
              f"fragmenting a message; point --stencil at a denser mesh to exercise the regression")

    print("passed")


if __name__ == "__main__":
    sys.exit(main())
