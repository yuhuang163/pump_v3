#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
BAJ文件转TXT数据文件工具
将BAJ文件转换为十六进制格式的文本文件，用于嵌入式系统
"""

import os
import sys
import argparse
import glob


def baj_to_txt(baj_path, output_dir=None, output_filename=None):
    """
    将BAJ文件转换为TXT格式的数据文件
    
    Args:
        baj_path: BAJ文件路径
        output_dir: 输出目录，如果为None则使用BAJ文件所在目录
        output_filename: 输出文件名，如果为None则使用BAJ文件名（替换扩展名为.txt）
    """
    # 检查输入文件是否存在
    if not os.path.exists(baj_path):
        print(f"错误: 文件不存在 - {baj_path}")
        return False
    
    # 读取BAJ文件（二进制模式）
    try:
        with open(baj_path, 'rb') as f:
            baj_data = f.read()
    except Exception as e:
        print(f"错误: 无法读取文件 - {e}")
        return False
    
    # 确定输出路径
    if output_dir is None:
        output_dir = os.path.dirname(baj_path)
    
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    
    # 确定输出文件名
    if output_filename is None:
        base_name = os.path.splitext(os.path.basename(baj_path))[0]
        output_filename = f"{base_name}.txt"
    
    output_path = os.path.join(output_dir, output_filename)
    
    # 转换为十六进制格式（C数组格式，每行末尾添加逗号）
    bytes_per_line = 16
    lines = []
    
    for i in range(0, len(baj_data), bytes_per_line):
        chunk = baj_data[i:i + bytes_per_line]
        hex_values = [f"0x{b:02x}" for b in chunk]
        line = ','.join(hex_values) + ','
        lines.append(line)
    
    # 写入文件
    try:
        with open(output_path, 'w') as f:
            f.write('\n'.join(lines))
        print(f"成功: 已转换 {len(baj_data)} 字节")
        print(f"输出文件: {output_path}")
        return True
    except Exception as e:
        print(f"错误: 无法写入文件 - {e}")
        return False


def batch_convert(input_path, output_dir=None, recursive=False):
    """
    批量转换BAJ文件
    
    Args:
        input_path: 输入文件或目录路径
        output_dir: 输出目录，如果为None则使用BAJ文件所在目录
        recursive: 是否递归处理子目录
    """
    if os.path.isfile(input_path):
        # 单个文件
        return baj_to_txt(input_path, output_dir, None)
    elif os.path.isdir(input_path):
        # 目录，查找所有BAJ文件
        if recursive:
            pattern = os.path.join(input_path, '**', '*.baj')
        else:
            pattern = os.path.join(input_path, '*.baj')
        
        baj_files = glob.glob(pattern, recursive=recursive)
        
        if not baj_files:
            print(f"错误: 在目录 {input_path} 中未找到BAJ文件")
            return False
        
        print(f"找到 {len(baj_files)} 个BAJ文件，开始批量转换...")
        print("-" * 60)
        
        success_count = 0
        fail_count = 0
        
        for baj_file in sorted(baj_files):
            # 如果指定了输出目录，保持相对路径结构
            file_output_dir = output_dir
            if output_dir and recursive:
                # 计算相对路径，保持目录结构
                rel_path = os.path.relpath(os.path.dirname(baj_file), input_path)
                if rel_path != '.':
                    file_output_dir = os.path.join(output_dir, rel_path)
            
            if baj_to_txt(baj_file, file_output_dir, None):
                success_count += 1
            else:
                fail_count += 1
            print()  # 空行分隔
        
        print("-" * 60)
        print(f"批量转换完成: 成功 {success_count} 个, 失败 {fail_count} 个")
        
        return fail_count == 0
    else:
        print(f"错误: 路径不存在 - {input_path}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description='将BAJ文件转换为TXT格式的数据文件（支持单文件和批量转换）',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 转换单个文件
  %(prog)s input.baj
  %(prog)s input.baj -o output_dir
  %(prog)s input.baj -o output_dir -n output.txt
  
  # 批量转换目录下所有BAJ文件
  %(prog)s out_img -o output_dir
  %(prog)s out_img -o output_dir -r
        """
    )
    
    parser.add_argument(
        'input_path',
        help='输入的BAJ文件路径或包含BAJ文件的目录路径'
    )
    
    parser.add_argument(
        '-o', '--output-dir',
        dest='output_dir',
        help='输出目录（默认：BAJ文件所在目录）'
    )
    
    parser.add_argument(
        '-n', '--name',
        dest='output_filename',
        help='输出文件名（仅单文件模式有效，默认：BAJ文件名.txt）'
    )
    
    parser.add_argument(
        '-r', '--recursive',
        dest='recursive',
        action='store_true',
        default=False,
        help='递归处理子目录（仅目录模式有效）'
    )
    
    args = parser.parse_args()
    
    # 判断是文件还是目录
    if os.path.isfile(args.input_path):
        # 单文件模式
        if args.recursive:
            print("警告: -r/--recursive 参数仅对目录有效，已忽略")
        
        success = baj_to_txt(
            args.input_path,
            args.output_dir,
            args.output_filename
        )
    elif os.path.isdir(args.input_path):
        # 批量转换模式
        if args.output_filename:
            print("警告: -n/--name 参数仅对单文件模式有效，已忽略")
        
        success = batch_convert(
            args.input_path,
            args.output_dir,
            args.recursive
        )
    else:
        print(f"错误: 路径不存在 - {args.input_path}")
        success = False
    
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
