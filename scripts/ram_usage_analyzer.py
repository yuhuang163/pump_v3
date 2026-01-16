#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
RAM使用情况分析工具
用于分析map文件中RAM各个模块的内存消耗细节
"""

import re
import sys
import os
from collections import defaultdict


class RAMAnalyzer:
    def __init__(self, map_file_path):
        self.map_file_path = map_file_path
        self.ram_sections = defaultdict(lambda: defaultdict(int))
        self.module_usage = defaultdict(int)
        self.section_details = defaultdict(list)
        self.file_usage = defaultdict(int)
        self.top_n = 50
        self.target_module = None
        
    def parse_map_file(self):
        """解析map文件"""
        print(f"正在解析map文件: {self.map_file_path}")
        
        if not os.path.exists(self.map_file_path):
            print(f"错误: 文件不存在 - {self.map_file_path}")
            return False
            
        try:
            with open(self.map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            # 查找RAM相关的段
            self._parse_ram_sections(content)
            
            return True
        except Exception as e:
            print(f"解析文件时出错: {e}")
            return False
    
    def _parse_ram_sections(self, content):
        """解析RAM相关段"""
        # 匹配RAM相关的段，包括：
        # .data, .bss, .ram_text, .sram, .ramx 等
        # 但排除PSRAM相关的段
        
        # 段名模式：包含ram/data/bss的段
        section_pattern = re.compile(
            r'^\s*(\.\S*(?:ram|data|bss|sram)\S*)\s+'  # 段名
            r'(0x[0-9a-fA-F]+)\s+'      # 地址
            r'(0x[0-9a-fA-F]+)\s+'      # 大小
            r'(.+)$',                    # 文件路径
            re.MULTILINE | re.IGNORECASE
        )
        
        for match in section_pattern.finditer(content):
            section_name = match.group(1)
            address = match.group(2)
            size_hex = match.group(3)
            file_path = match.group(4).strip()
            
            # 排除PSRAM相关的段
            if 'psram' in section_name.lower():
                continue
            
            size = int(size_hex, 16)
            
            if size > 0:
                # 提取模块名
                module_name = self._extract_module_name(file_path)
                
                self.ram_sections[section_name][module_name] += size
                self.module_usage[module_name] += size
                self.file_usage[file_path] += size
                self.section_details[section_name].append({
                    'file': file_path,
                    'size': size,
                    'address': address,
                    'module': module_name
                })
        
        # 查找Memory Configuration部分
        self._parse_memory_config(content)
    
    def _parse_memory_config(self, content):
        """解析Memory Configuration部分"""
        # 只匹配RAM相关区域，排除PSRAM
        memory_pattern = re.compile(
            r'^\s*(RAM\S*|SRAM\S*|FRAMX)\s*:\s+'  # 内存区域名（排除PSRAM开头的）
            r'(\d+)\s*B\s+'                        # 使用大小（字节）
            r'([\d.]+)\s*(MB|KB|GB)\s+'           # 区域大小
            r'([\d.]+)%',                          # 使用百分比
            re.MULTILINE | re.IGNORECASE
        )
        
        print("\n" + "="*80)
        print("RAM内存区域总览")
        print("="*80)
        
        found_memory_info = False
        for match in memory_pattern.finditer(content):
            region_name = match.group(1)
            
            # 跳过PSRAM相关区域
            if 'psram' in region_name.lower():
                continue
                
            found_memory_info = True
            used_size = int(match.group(2))
            region_size = float(match.group(3))
            size_unit = match.group(4)
            percentage = float(match.group(5))
            
            # 转换为KB显示
            used_kb = used_size / 1024
            
            print(f"{region_name:15s}: {used_kb:10.2f} KB / {region_size:8.2f} {size_unit}  ({percentage:6.2f}% 已使用)")
        
        if not found_memory_info:
            print("未找到Memory Configuration信息")
    
    def _extract_module_name(self, file_path):
        """从文件路径中提取模块名"""
        file_path = file_path.strip()
        
        if '/' in file_path:
            parts = file_path.split('/')
            
            # 查找关键目录名
            for i, part in enumerate(parts):
                if part in ['platform', 'drivers', 'services', 'apps', 'thirdparty', 
                           'utils', 'tests', 'rtos', 'bthost', 'lib', 'root_source',
                           'overlay', 'external']:
                    if i + 1 < len(parts):
                        return f"{part}/{parts[i+1]}"
                    return part
            
            # 如果包含src或include，使用上一级目录
            if 'src' in parts or 'include' in parts:
                idx = parts.index('src') if 'src' in parts else parts.index('include')
                if idx > 0:
                    return parts[idx - 1]
            
            # 使用倒数第二级目录
            if len(parts) >= 2:
                return parts[-2]
            
            return parts[-1]
        
        return file_path
    
    def generate_report(self):
        """生成分析报告"""
        if not self.module_usage:
            print("\n未找到RAM使用信息")
            return
        
        print("\n" + "="*80)
        print("RAM模块使用情况统计")
        print("="*80)
        
        # 按使用量排序
        sorted_modules = sorted(self.module_usage.items(), key=lambda x: x[1], reverse=True)
        
        total_size = sum(self.module_usage.values())
        
        print(f"\n{'模块名':<40s} {'大小(Bytes)':<15s} {'大小(KB)':<12s} {'占比':<10s}")
        print("-" * 80)
        
        for module, size in sorted_modules:
            size_kb = size / 1024
            percentage = (size / total_size * 100) if total_size > 0 else 0
            print(f"{module:<40s} {size:<15d} {size_kb:<12.2f} {percentage:>6.2f}%")
        
        print("-" * 80)
        total_kb = total_size / 1024
        total_mb = total_size / (1024 * 1024)
        print(f"{'总计':<40s} {total_size:<15d} {total_kb:<12.2f} ({total_mb:.2f} MB)")
        
        # 按段分类统计
        if self.ram_sections:
            print("\n" + "="*80)
            print("RAM段分类统计")
            print("="*80)
            
            for section_name in sorted(self.ram_sections.keys()):
                section_total = sum(self.ram_sections[section_name].values())
                section_kb = section_total / 1024
                print(f"\n{section_name}: {section_kb:.2f} KB")
                print("-" * 80)
                
                # 显示该段中的top模块
                sorted_section_modules = sorted(
                    self.ram_sections[section_name].items(),
                    key=lambda x: x[1],
                    reverse=True
                )[:10]
                
                for module, size in sorted_section_modules:
                    size_kb = size / 1024
                    percentage = (size / section_total * 100) if section_total > 0 else 0
                    print(f"  {module:<38s} {size_kb:>10.2f} KB  {percentage:>6.2f}%")
        
        # 详细的文件级别统计
        self._print_file_details()
    
    def _print_file_details(self):
        """打印详细的文件级别统计"""
        if not self.file_usage:
            return
        
        print("\n" + "="*80)
        print("RAM文件详细使用情况（按文件统计）")
        print("="*80)
        
        # 按使用量排序
        sorted_files = sorted(self.file_usage.items(), key=lambda x: x[1], reverse=True)
        
        # 如果指定了目标模块，只显示该模块的文件
        if self.target_module:
            print(f"\n只显示模块【{self.target_module}】的文件")
            sorted_files = [(f, s) for f, s in sorted_files 
                          if self._extract_module_name(f) == self.target_module]
        
        # 显示前N个文件
        display_count = min(self.top_n, len(sorted_files))
        print(f"\n{'文件路径':<70s} {'大小(KB)':<12s} {'大小(Bytes)':<15s}")
        print("-" * 100)
        
        for file_path, size in sorted_files[:display_count]:
            size_kb = size / 1024
            display_path = file_path if len(file_path) <= 70 else '...' + file_path[-67:]
            print(f"{display_path:<70s} {size_kb:>10.2f}   {size:>12d}")
        
        if len(sorted_files) > display_count:
            print(f"\n... 还有 {len(sorted_files) - display_count} 个文件未显示")
        
        # 按模块分组显示详细文件
        print("\n" + "="*80)
        print("按模块分组的文件详情（每个模块显示前10个文件）")
        print("="*80)
        
        # 按模块分组
        module_files = defaultdict(list)
        for section_name, details in self.section_details.items():
            for detail in details:
                module = detail.get('module', 'unknown')
                module_files[module].append({
                    'file': detail['file'],
                    'size': detail['size'],
                    'section': section_name
                })
        
        # 按模块使用量排序
        sorted_modules = sorted(self.module_usage.items(), key=lambda x: x[1], reverse=True)
        
        for module, total_size in sorted_modules[:10]:
            print(f"\n【{module}】 总计: {total_size/1024:.2f} KB")
            print("-" * 100)
            
            # 获取该模块的所有文件
            files = module_files[module]
            # 按文件合并统计
            file_stats = defaultdict(int)
            for f in files:
                file_stats[f['file']] += f['size']
            
            # 排序并显示前10个
            sorted_module_files = sorted(file_stats.items(), key=lambda x: x[1], reverse=True)
            
            print(f"  {'文件':<65s} {'大小(KB)':<12s} {'大小(Bytes)':<12s}")
            for file_path, size in sorted_module_files[:10]:
                size_kb = size / 1024
                display_path = file_path if len(file_path) <= 65 else '...' + file_path[-62:]
                print(f"  {display_path:<65s} {size_kb:>10.2f}   {size:>10d}")
            
            if len(sorted_module_files) > 10:
                print(f"  ... 还有 {len(sorted_module_files) - 10} 个文件")
    
    def export_csv(self, output_file):
        """导出为CSV文件"""
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write("模块名,大小(Bytes),大小(KB),大小(MB)\n")
                
                sorted_modules = sorted(self.module_usage.items(), key=lambda x: x[1], reverse=True)
                
                for module, size in sorted_modules:
                    size_kb = size / 1024
                    size_mb = size / (1024 * 1024)
                    f.write(f"{module},{size},{size_kb:.2f},{size_mb:.4f}\n")
            
            print(f"\n已导出CSV文件: {output_file}")
        except Exception as e:
            print(f"导出CSV文件失败: {e}")


def main():
    if len(sys.argv) < 2:
        print("用法: python ram_usage_analyzer.py <map文件路径> [输出CSV文件] [--module=模块名]")
        print("\n示例:")
        print("  python ram_usage_analyzer.py /path/to/project.map")
        print("  python ram_usage_analyzer.py /path/to/project.map output.csv")
        print("  python ram_usage_analyzer.py /path/to/project.map --module=platform/main")
        print("\n选项:")
        print("  --module=NAME    只显示指定模块的详细信息")
        print("  --top=N          显示前N个大文件（默认50）")
        sys.exit(1)
    
    map_file = sys.argv[1]
    csv_file = None
    target_module = None
    top_n = 50
    
    # 解析参数
    for arg in sys.argv[2:]:
        if arg.startswith('--module='):
            target_module = arg.split('=', 1)[1]
        elif arg.startswith('--top='):
            top_n = int(arg.split('=', 1)[1])
        elif not arg.startswith('--'):
            csv_file = arg
    
    analyzer = RAMAnalyzer(map_file)
    analyzer.top_n = top_n
    analyzer.target_module = target_module
    
    if analyzer.parse_map_file():
        analyzer.generate_report()
        
        if csv_file:
            analyzer.export_csv(csv_file)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()
