#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
PSRAM使用情况分析工具
用于分析map文件中PSRAM各个模块的内存消耗细节
"""

import re
import sys
import os
from collections import defaultdict
from pathlib import Path


class PSRAMAnalyzer:
    def __init__(self, map_file_path):
        self.map_file_path = map_file_path
        self.psram_sections = defaultdict(lambda: defaultdict(int))
        self.module_usage = defaultdict(int)
        self.section_details = defaultdict(list)
        self.file_usage = defaultdict(int)  # 新增：按文件统计
        self.top_n = 50  # 显示前N个文件
        self.target_module = None  # 目标模块
        
    def parse_map_file(self):
        """解析map文件"""
        print(f"正在解析map文件: {self.map_file_path}")
        
        if not os.path.exists(self.map_file_path):
            print(f"错误: 文件不存在 - {self.map_file_path}")
            return False
            
        try:
            with open(self.map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            # 查找PSRAM相关的段
            self._parse_psram_sections(content)
            
            return True
        except Exception as e:
            print(f"解析文件时出错: {e}")
            return False
    
    def _parse_psram_sections(self, content):
        """解析PSRAM相关段"""
        # 匹配PSRAM相关的段，格式如：
        # .psram_text    0x0000000038000000    0x12345  path/to/file.o
        # .psram_data    0x0000000038100000    0x678    path/to/file.o
        
        # 段名模式：包含psram的段
        section_pattern = re.compile(
            r'^\s*(\.\S*psram\S*)\s+'  # 段名（包含psram）
            r'(0x[0-9a-fA-F]+)\s+'      # 地址
            r'(0x[0-9a-fA-F]+)\s+'      # 大小
            r'(.+)$',                    # 文件路径
            re.MULTILINE
        )
        
        # 也匹配PSRAM_NC等变体
        alt_pattern = re.compile(
            r'^\s*(\.\S+)\s+'            # 段名
            r'(0x[0-9a-fA-F]+)\s+'      # 地址  
            r'(0x[0-9a-fA-F]+)\s+'      # 大小
            r'(.+)$',                    # 文件路径
            re.MULTILINE
        )
        
        for match in section_pattern.finditer(content):
            section_name = match.group(1)
            address = match.group(2)
            size_hex = match.group(3)
            file_path = match.group(4).strip()
            
            size = int(size_hex, 16)
            
            if size > 0:
                # 提取模块名（从文件路径中）
                module_name = self._extract_module_name(file_path)
                
                self.psram_sections[section_name][module_name] += size
                self.module_usage[module_name] += size
                self.file_usage[file_path] += size  # 新增：记录文件级别使用
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
        # 查找类似这样的输出：
        # Memory region         Used Size  Region Size  %age Used
        #            PSRAM:     3473216 B        16 MB     20.70%
        #         PSRAM_NC:     3718976 B        16 MB     22.17%
        
        memory_pattern = re.compile(
            r'^\s*(PSRAM\S*)\s*:\s+'     # 内存区域名
            r'(\d+)\s*B\s+'               # 使用大小（字节）
            r'([\d.]+)\s*(MB|KB|GB)\s+'  # 区域大小
            r'([\d.]+)%',                 # 使用百分比
            re.MULTILINE | re.IGNORECASE
        )
        
        print("\n" + "="*80)
        print("PSRAM内存区域总览")
        print("="*80)
        
        found_memory_info = False
        for match in memory_pattern.finditer(content):
            found_memory_info = True
            region_name = match.group(1)
            used_size = int(match.group(2))
            region_size = float(match.group(3))
            size_unit = match.group(4)
            percentage = float(match.group(5))
            
            # 转换为MB显示
            used_mb = used_size / (1024 * 1024)
            
            print(f"{region_name:15s}: {used_mb:10.2f} MB / {region_size:8.2f} {size_unit}  ({percentage:6.2f}% 已使用)")
        
        if not found_memory_info:
            print("未找到Memory Configuration信息")
    
    def _extract_module_name(self, file_path):
        """从文件路径中提取模块名"""
        # 移除.o后缀
        file_path = file_path.strip()
        
        # 尝试从路径中提取有意义的模块名
        if '/' in file_path:
            parts = file_path.split('/')
            
            # 查找关键目录名
            for i, part in enumerate(parts):
                if part in ['platform', 'drivers', 'services', 'apps', 'thirdparty', 
                           'utils', 'tests', 'rtos', 'bthost', 'lib']:
                    if i + 1 < len(parts):
                        return f"{part}/{parts[i+1]}"
                    return part
            
            # 如果包含src或include，使用上一级目录
            if 'src' in parts or 'include' in parts:
                idx = parts.index('src') if 'src' in parts else parts.index('include')
                if idx > 0:
                    return parts[idx - 1]
            
            # 使用倒数第二级目录（如果存在）
            if len(parts) >= 2:
                return parts[-2]
            
            return parts[-1]
        
        return file_path
    
    def generate_report(self):
        """生成分析报告"""
        if not self.module_usage:
            print("\n未找到PSRAM使用信息")
            return
        
        print("\n" + "="*80)
        print("PSRAM模块使用情况统计")
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
        if self.psram_sections:
            print("\n" + "="*80)
            print("PSRAM段分类统计")
            print("="*80)
            
            for section_name in sorted(self.psram_sections.keys()):
                section_total = sum(self.psram_sections[section_name].values())
                section_mb = section_total / (1024 * 1024)
                print(f"\n{section_name}: {section_mb:.2f} MB")
                print("-" * 80)
                
                # 显示该段中的top模块
                sorted_section_modules = sorted(
                    self.psram_sections[section_name].items(),
                    key=lambda x: x[1],
                    reverse=True
                )[:10]  # 只显示前10个
                
                for module, size in sorted_section_modules:
                    size_kb = size / 1024
                    percentage = (size / section_total * 100) if section_total > 0 else 0
                    print(f"  {module:<38s} {size_kb:>10.2f} KB  {percentage:>6.2f}%")
        
        # 新增：详细的文件级别统计
        self._print_file_details()
    
    def _print_file_details(self):
        """打印详细的文件级别统计"""
        if not self.file_usage:
            return
        
        print("\n" + "="*80)
        print("PSRAM文件详细使用情况（按文件统计）")
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
            # 提取文件名
            file_name = file_path.split('/')[-1] if '/' in file_path else file_path
            # 如果路径太长，截断显示
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
        
        for module, total_size in sorted_modules[:10]:  # 只显示前10个模块
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
                file_name = file_path.split('/')[-1] if '/' in file_path else file_path
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
        print("用法: python psram_usage_analyzer.py <map文件路径> [输出CSV文件] [--module=模块名]")
        print("\n示例:")
        print("  python psram_usage_analyzer.py /path/to/project.map")
        print("  python psram_usage_analyzer.py /path/to/project.map output.csv")
        print("  python psram_usage_analyzer.py /path/to/project.map --module=platform/main")
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
    
    analyzer = PSRAMAnalyzer(map_file)
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
