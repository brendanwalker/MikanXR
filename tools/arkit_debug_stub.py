"""Stand-in for the MikanARStreamer app's debug-channel client.

Exercises the editor's ARKit debug channel without a phone: it completes the
handshake, pushes a couple of diagnostic lines, and answers commands sent by
`arkit send`. Useful for testing the channel itself, and for reproducing a
protocol problem without a device on the bench.

Run the editor with the channel enabled, then this stub, then drive it:

    build/src/Editor/Release/Mikan.exe -arkitDebugChannel
    python tools/arkit_debug_stub.py
    python tools/automate.py "arkit status" "arkit send ping" "log tail 20 info"

Commands answered: ping, stats, empty (a zero-line reply), and silent, which
deliberately never answers so the `arkit send` timeout can be exercised.
Anything else gets a one-line "unknown command".
"""
import argparse
import socket
import sys

PROTOCOL_VERSION = 1


def build_reply(lines):
    """Frame a reply the way the channel expects: a count line, then the lines."""
    payload = f"reply {len(lines)}\n"
    for line in lines:
        payload += f"{line}\n"
    return payload.encode()


def answer(command):
    """Return the bytes to send for a command, or None to stay silent."""
    if command == "ping":
        return build_reply(["pong"])
    if command == "stats":
        return build_reply(["fps 29.9", "dropped 0", "encoded 812"])
    if command == "empty":
        return build_reply([])
    if command == "silent":
        return None
    return build_reply(["unknown command"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1", help="editor host (default 127.0.0.1)")
    parser.add_argument("--port", type=int, default=21121, help="debug channel port (default 21121)")
    parser.add_argument("--name", default="StubPhone", help="device name reported in the handshake")
    parser.add_argument("--timeout", type=float, default=600.0, help="idle seconds before giving up")
    args = parser.parse_args()

    try:
        sock = socket.create_connection((args.host, args.port), timeout=10)
    except OSError as error:
        print(f"could not connect to {args.host}:{args.port}: {error}")
        print("is the editor running with -arkitDebugChannel?")
        return 1

    sock.settimeout(args.timeout)
    stream = sock.makefile("rwb")

    stream.write(f"hello {PROTOCOL_VERSION} {args.name}\n".encode())
    stream.flush()

    ack = stream.readline().decode().strip()
    if ack != f"ok {PROTOCOL_VERSION}":
        print(f"handshake rejected: {ack!r}")
        return 1
    print(f"connected to {args.host}:{args.port} as {args.name}")

    stream.write(b"log warning stub encode stalled frameSeq=1234\n")
    stream.write(b"log info stub steady state 30fps\n")
    stream.flush()

    while True:
        try:
            line = stream.readline()
        except socket.timeout:
            print("idle timeout, exiting")
            return 0

        if not line:
            print("editor closed the connection")
            return 0

        text = line.decode().strip()
        if not text.startswith("cmd "):
            continue

        command = text[4:]
        print(f"command: {command}")

        payload = answer(command)
        if payload is None:
            print("  (staying silent so the send times out)")
            continue

        stream.write(payload)
        stream.flush()


if __name__ == "__main__":
    sys.exit(main())
