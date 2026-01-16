#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
解析资源包bin文件，显示所有资源的ID信息
"""

import struct
import sys
from pathlib import Path

# 资源包结构定义
IMG_RES_MAGIC = 0x52534547  # "GESR"
IMG_RES_MD5_SIZE = 16

def parse_resource_pack(bin_file):
    """解析资源包文件"""
    with open(bin_file, 'rb') as f:
        # 读取头部描述 (20字节: img_res_head_desc_t，不包括reserved)
        head_data = f.read(20)
        magic, head_crc16, flags, res_type, lang_id, version, res_count, total_size = struct.unpack('<IHHHHHHI', head_data)
        
        # 读取reserved字段 (16字节)
        reserved = struct.unpack('<IIII', f.read(16))
        
        # 验证魔数
        if magic != IMG_RES_MAGIC:
            print(f"❌ 错误: 无效的资源包文件（魔数不匹配: 0x{magic:08X} != 0x{IMG_RES_MAGIC:08X}）")
            return False
        
        print(f"📦 资源包信息:")
        print(f"  魔数: 0x{magic:08X} ({'GESR' if magic == IMG_RES_MAGIC else 'INVALID'})")
        print(f"  资源类型: {res_type}")
        print(f"  语言ID: {lang_id}")
        print(f"  版本: 0x{version:04X}")
        print(f"  资源数量: {res_count}")
        print(f"  总大小: {total_size} bytes")
        print()
        
        # 跳过MD5哈希 (16 + 16 = 32字节)
        f.read(IMG_RES_MD5_SIZE * 2)
        
        # 读取所有资源的obj_info
        print(f"📋 资源列表 (共 {res_count} 个):")
        print(f"{'ID':<6} {'Width':<8} {'Height':<8} {'Size':<12} {'Offset':<12} {'CRC16':<8}")
        print("-" * 70)
        
        animation_ids = []
        
        for i in range(res_count):
            # 读取img_res_obj_info_t (20字节)
            obj_data = f.read(20)
            img_id, crc16, width, height, x_offset, y_offset, offset_addr, data_size = struct.unpack('<HHHHHHII', obj_data)
            
            # 判断是否为动画资源 (ID 300-349)
            is_animation = 300 <= img_id <= 349
            if is_animation:
                animation_ids.append(img_id)
            
            print(f"{img_id:<6} {width:<8} {height:<8} {data_size:<12} 0x{offset_addr:08X} 0x{crc16:04X}")
        
        print()
        print(f"🎬 动画资源统计:")
        print(f"  动画资源ID范围: {min(animation_ids) if animation_ids else 'N/A'} - {max(animation_ids) if animation_ids else 'N/A'}")
        print(f"  动画资源数量: {len(animation_ids)}")
        if animation_ids:
            print(f"  动画资源ID列表: {animation_ids}")
        
        return True

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("使用方法: python3 dump_resource_ids.py <bin_file>")
        print("示例: python3 dump_resource_ids.py ../bin/V3_RES_V200_*.bin")
        sys.exit(1)
    
    bin_file = sys.argv[1]
    if not Path(bin_file).exists():
        print(f"❌ 错误: 文件不存在: {bin_file}")
        sys.exit(1)
    
    parse_resource_pack(bin_file)

