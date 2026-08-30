"""Measure ArUco detection and pose stability from a live ARKit video stream.

Two jobs. It is the OpenCV arm of the ARKit-image-tracking comparison, and it is
a prototype of the detection MikanXR itself needs to do: take a streamed ARKit
frame, find the marker, and solve for where it sits. Everything needed is already
on the wire, so this reads it from there rather than assuming anything.

The intrinsics come from the RTP header extension rather than from a calibration
file, because ARKit reports them per frame and they are what the stream was
actually captured with. Frames are decoded by GStreamer to JPEG, because the
OpenCV wheels on this machine are built without GStreamer support.

    python tools/aruco_pose_measure.py --seconds 20 --marker-mm 100

Reports, with the phone and marker stationary:
  - the fraction of frames the marker was found in
  - how far the solved position wanders while nothing physically moves
"""

import argparse
import glob
import os
import shutil
import socket
import struct
import subprocess
import sys
import tempfile

import cv2
import numpy as np

GST = r"C:\gstreamer\1.0\mingw_x86_64\bin\gst-launch-1.0.exe"
CAPS = "application/x-rtp,media=video,encoding-name=H264,payload=96"

# Pose payload layout, mirroring ARKitPoseInRTPPayload: the RTP header is 12
# bytes, the two-byte-form extension header 6, then frameSeq(4) and
# captureTimestampUs(8) before the 16 transform floats and the intrinsics.
POSE_OFFSET = 12 + 6
INTRINSICS_OFFSET = POSE_OFFSET + 4 + 8 + 64


def read_intrinsics(port, timeout=10.0):
    """Pull fx, fy, cx, cy and the capture resolution off one RTP packet."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4 * 1024 * 1024)
    sock.bind(("0.0.0.0", port))
    sock.settimeout(timeout)
    try:
        for _ in range(2000):
            packet, _ = sock.recvfrom(65535)
            if len(packet) < INTRINSICS_OFFSET + 24:
                continue
            fx, fy, cx, cy, width, height = struct.unpack(
                ">6f", packet[INTRINSICS_OFFSET:INTRINSICS_OFFSET + 24])
            if fx > 0 and width > 0:
                return fx, fy, cx, cy, width, height
    except socket.timeout:
        pass
    finally:
        sock.close()
    return None


def capture_frames(port, seconds, directory):
    """Decode the stream to JPEGs. Software decode keeps this independent of the
    hardware decoder's own behaviour, which is not what is being measured."""
    pipeline = [
        GST, "udpsrc", f"port={port}", f"caps={CAPS}", "buffer-size=4194304",
        "!", "rtpjitterbuffer", "latency=200",
        "!", "rtph264depay", "!", "h264parse", "!", "openh264dec",
        "!", "videoconvert", "!", "jpegenc",
        # Forward slashes even on Windows: multifilesink rejects a backslash
        # path with "Permission denied", which reads like a filesystem problem
        # rather than the parsing one it is.
        "!", "multifilesink",
        f"location={os.path.join(directory, '%05d.jpg').replace(os.sep, '/')}",
    ]
    try:
        subprocess.run(pipeline, timeout=seconds, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        pass
    return sorted(glob.glob(os.path.join(directory, "*.jpg")))


def measure(paths, camera_matrix, marker_mm, dictionary_name):
    dictionary = cv2.aruco.getPredefinedDictionary(
        getattr(cv2.aruco, dictionary_name))
    detector = cv2.aruco.ArucoDetector(dictionary, cv2.aruco.DetectorParameters())

    half = marker_mm / 2000.0  # millimetres to metres, half-edge
    # Marker-local corners in the order detectMarkers returns them.
    object_points = np.array([
        [-half, half, 0], [half, half, 0], [half, -half, 0], [-half, -half, 0],
    ], dtype=np.float32)
    # ARKit reports no lens distortion, so the frames are already rectilinear.
    distortion = np.zeros((5, 1), dtype=np.float32)

    found = 0
    positions = {}
    for path in paths:
        image = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        if image is None:
            continue
        corners, ids, _ = detector.detectMarkers(image)
        if ids is None:
            continue
        found += 1
        for corner, marker_id in zip(corners, ids.flatten()):
            ok, rvec, tvec = cv2.solvePnP(
                object_points, corner.reshape(4, 2), camera_matrix, distortion,
                flags=cv2.SOLVEPNP_IPPE_SQUARE)
            if ok:
                positions.setdefault(int(marker_id), []).append(tvec.flatten())
    return found, positions


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=27015)
    parser.add_argument("--seconds", type=float, default=20.0)
    parser.add_argument("--marker-mm", type=float, default=100.0)
    parser.add_argument("--dictionary", default="DICT_6X6_250")
    args = parser.parse_args()

    intrinsics = read_intrinsics(args.port)
    if intrinsics is None:
        print("no RTP packets on port", args.port, "- is the phone streaming?")
        return 1
    fx, fy, cx, cy, width, height = intrinsics
    print(f"intrinsics fx={fx:.1f} fy={fy:.1f} cx={cx:.1f} cy={cy:.1f} "
          f"capture={int(width)}x{int(height)}")

    camera_matrix = np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]], dtype=np.float32)

    directory = tempfile.mkdtemp(prefix="aruco_frames_")
    try:
        paths = capture_frames(args.port, args.seconds, directory)
        if not paths:
            print("no frames decoded")
            return 1
        # The decoder must be producing frames at the capture resolution, or the
        # intrinsics above do not describe them and every pose would be wrong.
        sample = cv2.imread(paths[0])
        if sample is not None and sample.shape[1] != int(width):
            print(f"WARNING decoded {sample.shape[1]}x{sample.shape[0]} but "
                  f"intrinsics describe {int(width)}x{int(height)}")

        found, positions = measure(paths, camera_matrix, args.marker_mm, args.dictionary)
        rate = found / len(paths) if paths else 0
        print(f"frames {len(paths)}  detected {found}  rate {rate:.1%}")

        for marker_id, samples in sorted(positions.items()):
            array = np.array(samples)
            mean = array.mean(axis=0)
            sd = array.std(axis=0) * 1000.0
            spread = np.linalg.norm(array - mean, axis=1).max() * 1000.0
            print(f"  id {marker_id}: n={len(array)} "
                  f"distance={np.linalg.norm(mean):.3f}m "
                  f"jitterMM=({sd[0]:.2f}, {sd[1]:.2f}, {sd[2]:.2f}) "
                  f"maxDeviationMM={spread:.2f}")
    finally:
        shutil.rmtree(directory, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
