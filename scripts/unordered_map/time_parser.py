import pandas as pd
import re
import matplotlib.pyplot as plt
import io
import os

def parse_test_stats(stats_file):
    """Parses the experiment statistics from test_stats.txt."""
    with open(stats_file, 'r') as f:
        content = f.read()
    
    # Split by experiment blocks
    exp_blocks = re.split(r'=== Running experiment number (\d+) ===', content)
    stats_dict = {}
    
    # exp_blocks[0] is header/empty, then it alternates: [id, content, id, content...]
    for i in range(1, len(exp_blocks), 2):
        exp_id = int(exp_blocks[i])
        block_text = exp_blocks[i+1]
        
        # Extract specific metrics
        seed = re.search(r'Seed: (\d+)', block_text).group(1)
        modules = re.search(r'Number of modules: (\d+)', block_text).group(1)
        variables = re.search(r'Number of variables: (\d+)', block_text).group(1)
        depth = re.search(r'Tree depth: (\d+)', block_text).group(1)
        
        stats_dict[exp_id] = {
            'Metric': ['Seed', 'Number of Modules', 'Number of Variables', 'Tree Depth'],
            'Value': [int(seed), int(modules), int(variables), int(depth)]
        }
    return stats_dict

def process_all_files_with_stats(file_prefix, stats_file, file_range, output_excel):
    name_map = {'0': 'Variable', '1': 'Relations', '2': 'Default'}
    
    # Parse the stats file once
    all_stats = parse_test_stats(stats_file)
    
    with pd.ExcelWriter(output_excel, engine='xlsxwriter') as writer:
        for i in file_range:
            file_path = f"{file_prefix}{i}.txt"
            sheet_name = f"Divider_{i}"
            
            if not os.path.exists(file_path):
                continue

            # --- 1. PARSE PERFORMANCE DATA ---
            with open(file_path, 'r') as f:
                content = f.read()

            parts = re.split(r'==+Running experiment with MAP structure==+', content)
            data_list = []
            for part_idx, part in enumerate(parts):
                is_map = (part_idx == 1)
                suffix = " MAP" if is_map else ""
                core_blocks = re.split(r'Cores used:', part)
                for block in core_blocks[1:]:
                    core_match = re.search(r'^\s*(\d+)', block)
                    if not core_match: continue
                    core_val = int(core_match.group(1))
                    matches = re.findall(r"Divider used: (\d+).*?Average time to get module: ([\d.e-]+) seconds.*?Time: ([\d.e-]+)", block, re.DOTALL)
                    for div_id, avg_time, total_time in matches:
                        base_name = name_map.get(div_id)
                        data_list.append({
                            'Cores': core_val,
                            'Divider': base_name,
                            'IsMap': is_map,
                            'TimeCol': f"{base_name} Time{suffix}",
                            'AvgCol': f"{base_name} Get Time{suffix}",
                            'TimeVal': float(total_time),
                            'AvgVal': float(avg_time)
                        })

            df = pd.DataFrame(data_list)
            
            # --- 2. WRITE TABLES TO EXCEL ---
            table_time = df.pivot(index='Cores', columns='TimeCol', values='TimeVal')
            table_avg = df.pivot(index='Cores', columns='AvgCol', values='AvgVal')
            
            time_cols = ['Variable Time', 'Relations Time', 'Default Time', 'Variable Time MAP', 'Relations Time MAP', 'Default Time MAP']
            avg_cols = ['Variable Get Time', 'Relations Get Time', 'Default Get Time', 'Variable Get Time MAP', 'Relations Get Time MAP', 'Default Get Time MAP']
            
            table_time.reindex(columns=time_cols).to_excel(writer, sheet_name=sheet_name, startrow=0)
            table_avg.reindex(columns=avg_cols).to_excel(writer, sheet_name=sheet_name, startrow=10)
            
            # Write the new Stats Table at row 20
            if i in all_stats:
                df_stats = pd.DataFrame(all_stats[i])
                df_stats.to_excel(writer, sheet_name=sheet_name, startrow=20, index=False)
            
            worksheet = writer.sheets[sheet_name]

            # --- 3. ADD GRAPHS (Straight Lines) ---
            def insert_plot(divider, metric_col, title, y_label, excel_pos):
                plt.figure(figsize=(5, 3.5))
                for is_map, label, style in [(False, 'Standard', 'o-'), (True, 'MAP', 's--')]:
                    subset = df[(df['Divider'] == divider) & (df['IsMap'] == is_map)].sort_values('Cores')
                    plt.plot(subset['Cores'], subset[metric_col], style, label=label)

                plt.title(title, fontsize=10)
                plt.xlabel('Cores', fontsize=9)
                plt.ylabel(y_label, fontsize=9)
                plt.xticks([1, 2, 4, 8, 16])
                plt.grid(True, alpha=0.3)
                plt.legend(prop={'size': 8})
                
                img_data = io.BytesIO()
                plt.savefig(img_data, format='png', bbox_inches='tight')
                plt.close()
                worksheet.insert_image(excel_pos, f'{divider}_{metric_col}_{i}.png', {'image_data': img_data})

            divs = ['Variable', 'Relations', 'Default']
            for idx, div in enumerate(divs):
                insert_plot(div, 'TimeVal', f'{div}: Total Time', 'Seconds', f'I{1 + (idx * 18)}')
                insert_plot(div, 'AvgVal', f'{div}: Get Time', 'Seconds', f'O{1 + (idx * 18)}')

    print(f"Final report generated with statistics: {output_excel}")

# Usage
process_all_files_with_stats('script_output_divider_', 'test_stats.txt', range(8), 'Full_Performance_Report.xlsx')