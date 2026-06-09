#!/usr/bin/env python3
"""
comm_protocol_assemble_packet 组包逻辑测试脚本

模拟 BLE MTU 分片场景，验证 comm_protocol_parser.c 中的环形缓冲组包流程。
用法: python3 test_comm_protocol_assemble.py
"""

from __future__ import annotations

import struct
import sys
from dataclasses import dataclass, field
from typing import Callable, List, Optional, Tuple

# ── 协议常量（与 comm_protocol_defs.h 一致）────────────────────────────
COMM_PROTOCOL_SOF = 0x5CC5
COMM_PROTOCOL_HEADER_SIZE = 6
COMM_PROTOCOL_FOOTER_SIZE = 2
COMM_PROTOCOL_SERVICE_HEADER_SIZE = 4
COMM_PROTOCOL_TLV_HEADER_SIZE = 4
COMM_PROTOCOL_PAYLOAD_SIZE_MIN = COMM_PROTOCOL_HEADER_SIZE + COMM_PROTOCOL_FOOTER_SIZE
COMM_PROTOCOL_MAX_PAYLOAD_SIZE = 1024
COMM_PROTOCOL_OTA_SERVICE = 9
PACKET_ASSEMBLY_BUFFER_SIZE = 1024


def crc16_compute(data: bytes, crc: int = 0xFFFF) -> int:
    """与 trunk/root_source/utils/gernal_utils.c 中 crc16_compute 一致"""
    for b in data:
        crc = ((crc >> 8) & 0xFFFF) | ((crc << 8) & 0xFFFF)
        crc ^= b
        crc ^= (crc & 0xFF) >> 4
        crc ^= ((crc << 8) << 4) & 0xFFFF
        crc ^= (((crc & 0xFF) << 4) << 1) & 0xFFFF
    return crc & 0xFFFF


def build_frame(
    seq: int,
    frame_type: int,
    payload: bytes,
) -> bytes:
    """构造一帧合法的 comm_protocol 数据"""
    header = struct.pack("<HBBH", COMM_PROTOCOL_SOF, seq, frame_type, len(payload))
    body = header + payload
    checksum = crc16_compute(body)
    return body + struct.pack("<H", checksum)


def build_ota_request_frame(seq: int = 1, tlv_value: bytes = b"\x01\x02\x03\x04") -> bytes:
    """构造 OTA 服务请求帧（最小合法 Payload）"""
    tlv = struct.pack("<HH", 0x0001, len(tlv_value)) + tlv_value
    service = struct.pack("<HH", COMM_PROTOCOL_OTA_SERVICE, len(tlv)) + tlv
    return build_frame(seq, 0, service)


def split_chunks(data: bytes, mtu: int) -> List[bytes]:
    """按 MTU 大小切分，模拟 BLE 多次回调"""
    return [data[i : i + mtu] for i in range(0, len(data), mtu)]


@dataclass
class PacketAssemblyBuffer:
    """Python 版环形缓冲，逻辑对齐 comm_protocol_parser.c"""

    buffer: bytearray = field(default_factory=lambda: bytearray(PACKET_ASSEMBLY_BUFFER_SIZE))
    write_pos: int = 0
    read_pos: int = 0
    data_size: int = 0

    def reset(self) -> None:
        self.write_pos = 0
        self.read_pos = 0
        self.data_size = 0

    def write(self, data: bytes) -> int:
        if len(data) > PACKET_ASSEMBLY_BUFFER_SIZE - self.data_size:
            return -1
        for b in data:
            self.buffer[self.write_pos] = b
            self.write_pos = (self.write_pos + 1) % PACKET_ASSEMBLY_BUFFER_SIZE
        self.data_size += len(data)
        return 0

    def peek(self, offset: int) -> int:
        return self.buffer[(self.read_pos + offset) % PACKET_ASSEMBLY_BUFFER_SIZE]

    def read(self, length: int) -> bytes:
        out = bytearray(length)
        for i in range(length):
            out[i] = self.buffer[self.read_pos]
            self.read_pos = (self.read_pos + 1) % PACKET_ASSEMBLY_BUFFER_SIZE
        self.data_size -= length
        return bytes(out)


def comm_protocol_assemble_packet(
    asm: PacketAssemblyBuffer,
    data: bytes,
    callback: Callable[[bytes], None],
) -> int:
    """
    对齐 comm_protocol_assemble_packet 核心逻辑（不含超时）
    返回: 0=处理完毕, 1=需要更多数据, <0=错误
    """
    if not data:
        return -1
    if asm.write(data) < 0:
        asm.reset()
        return -2

    while asm.data_size > 0:
        if asm.data_size < COMM_PROTOCOL_HEADER_SIZE:
            return 1

        header = bytes(asm.peek(i) for i in range(COMM_PROTOCOL_HEADER_SIZE))
        sof = header[0] | (header[1] << 8)
        if sof != COMM_PROTOCOL_SOF:
            asm.read(1)
            continue

        payload_length = header[4] | (header[5] << 8)
        if payload_length < COMM_PROTOCOL_PAYLOAD_SIZE_MIN or payload_length > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
            asm.read(1)
            continue

        frame_size = COMM_PROTOCOL_HEADER_SIZE + payload_length + COMM_PROTOCOL_FOOTER_SIZE
        if asm.data_size < frame_size:
            return 1

        frame = asm.read(frame_size)
        checksum_expected = crc16_compute(frame[:-COMM_PROTOCOL_FOOTER_SIZE])
        checksum_received = frame[-2] | (frame[-1] << 8)
        if checksum_expected != checksum_received:
            continue

        callback(frame)

    return 0


def feed_chunks(
    chunks: List[bytes],
    callback: Callable[[bytes], None],
) -> Tuple[List[bytes], List[int]]:
    """按顺序喂入分片，返回拼出的完整帧和各次返回值"""
    asm = PacketAssemblyBuffer()
    frames: List[bytes] = []
    rets: List[int] = []

    def _cb(frame: bytes) -> None:
        frames.append(frame)

    for chunk in chunks:
        ret = comm_protocol_assemble_packet(asm, chunk, _cb)
        rets.append(ret)

    return frames, rets


# ── 测试用例 ──────────────────────────────────────────────────────────

def test_single_chunk() -> None:
    frame = build_ota_request_frame(seq=1)
    frames, _ = feed_chunks([frame], lambda f: None)
    assert len(frames) == 1, f"期望 1 帧, 实际 {len(frames)}"
    assert frames[0] == frame, "单包场景帧内容不一致"
    print("[PASS] 单包（无分片）")


def test_mtu_split() -> None:
    """模拟 MTU=200，帧长约 500 字节的分片组包"""
  # 构造较大 payload
    big_value = bytes(range(256)) * 2  # 512 字节 TLV value
    tlv = struct.pack("<HH", 0x0100, len(big_value)) + big_value
    service = struct.pack("<HH", COMM_PROTOCOL_OTA_SERVICE, len(tlv)) + tlv
    frame = build_frame(seq=2, frame_type=0, payload=service)

    mtu = 200
    chunks = split_chunks(frame, mtu)
    frames, rets = feed_chunks(chunks, lambda f: None)

    assert len(frames) == 1, f"期望拼出 1 帧, 实际 {len(frames)}"
    assert frames[0] == frame, "分片重组后帧内容不一致"
    assert rets[-1] == 0, f"最后一包应返回 0, 实际 {rets[-1]}"
    assert any(r == 1 for r in rets[:-1]), "中间分片应返回 1（需要更多数据）"
    print(f"[PASS] MTU={mtu} 分片组包 (帧长={len(frame)}, 分{len(chunks)}片)")


def test_small_mtu() -> None:
    """模拟极小 MTU（20 字节），接近真实 BLE 默认 ATT"""
    frame = build_ota_request_frame(seq=3, tlv_value=bytes(100))
    mtu = 20
    chunks = split_chunks(frame, mtu)
    frames, _ = feed_chunks(chunks, lambda f: None)
    assert len(frames) == 1 and frames[0] == frame
    print(f"[PASS] MTU={mtu} 小分片组包 (帧长={len(frame)}, 分{len(chunks)}片)")


def test_two_frames_back_to_back() -> None:
    """连续两帧粘包"""
    f1 = build_ota_request_frame(seq=10, tlv_value=b"\xAA")
    f2 = build_ota_request_frame(seq=11, tlv_value=b"\xBB")
    combined = f1 + f2
    frames, _ = feed_chunks(split_chunks(combined, 64), lambda f: None)
    assert len(frames) == 2
    assert frames[0] == f1 and frames[1] == f2
    print("[PASS] 连续两帧粘包 + 分片")


def test_invalid_sof_recovery() -> None:
    """帧前有脏字节，应滑动丢弃直到找到 SOF"""
    frame = build_ota_request_frame(seq=20)
    garbage = b"\xFF\xFE\x00"
    combined = garbage + frame
    frames, _ = feed_chunks([combined], lambda f: None)
    assert len(frames) == 1 and frames[0] == frame
    print("[PASS] 脏数据前缀 + SOF 滑动恢复")


def test_incomplete_then_complete() -> None:
    """先收到半包，再收到剩余半包"""
    frame = build_ota_request_frame(seq=30, tlv_value=bytes(80))
    mid = len(frame) // 2
    asm = PacketAssemblyBuffer()
    got: List[bytes] = []

    r1 = comm_protocol_assemble_packet(asm, frame[:mid], lambda f: got.append(f))
    r2 = comm_protocol_assemble_packet(asm, frame[mid:], lambda f: got.append(f))

    assert r1 == 1, f"半包应返回 1, 实际 {r1}"
    assert r2 == 0, f"补全后应返回 0, 实际 {r2}"
    assert len(got) == 1 and got[0] == frame
    print("[PASS] 半包等待 + 补全")


def hex_dump(data: bytes, label: str = "") -> None:
    prefix = f"{label}: " if label else ""
    print(prefix + data.hex(" "))


def demo_mtu_split() -> None:
    """打印一次完整的分片组包过程（便于肉眼对照）"""
    print("\n── 分片组包演示 ──")
    frame = build_ota_request_frame(seq=99, tlv_value=bytes(50))
    mtu = 20
    chunks = split_chunks(frame, mtu)
    print(f"完整帧 ({len(frame)} 字节):")
    hex_dump(frame, "  frame")

    asm = PacketAssemblyBuffer()
    assembled: List[bytes] = []

    for i, chunk in enumerate(chunks):
        ret = comm_protocol_assemble_packet(asm, chunk, lambda f: assembled.append(f))
        print(f"  第{i + 1}片 len={len(chunk):3d}  buf累计={asm.data_size:3d}  ret={ret}")
        hex_dump(chunk, "    chunk")

    if assembled:
        print(f"组包结果: 共 {len(assembled)} 帧, 长度={len(assembled[0])}")
        assert assembled[0] == frame


def main() -> int:
    tests = [
        test_single_chunk,
        test_mtu_split,
        test_small_mtu,
        test_two_frames_back_to_back,
        test_invalid_sof_recovery,
        test_incomplete_then_complete,
    ]

    failed = 0
    for t in tests:
        try:
            t()
        except AssertionError as e:
            print(f"[FAIL] {t.__name__}: {e}")
            failed += 1
        except Exception as e:
            print(f"[ERROR] {t.__name__}: {e}")
            failed += 1

    demo_mtu_split()

    print(f"\n{'=' * 40}")
    if failed:
        print(f"失败 {failed}/{len(tests)}")
        return 1
    print(f"全部通过 {len(tests)}/{len(tests)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
