import pandas as pd
import re
import matplotlib.pyplot as plt
import io
import os
import sys

def parse_all_averaged_data(output_dir, stats_file, file_range):
    name_map = {'0': 'Variable', '1': 'Relations', '2': 'Default'}
    all_perf_data = []
    
    for i in file_range:
        file_path = os.path.join(output_dir, f"script_output_divider_{i}.txt")
        if not os.path.exists(file_path): continue
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        parts = re.split(r'==+Running experiment with MAP structure==+', content)
        if len(parts) < 2: continue 
        
        core_blocks = re.split(r'Cores used:', parts[1])
        for block in core_blocks[1:]:
            core_match = re.search(r'^\s*(\d+)', block)
            if not core_match: continue
            core_val = int(core_match.group(1))
            
            matches = re.findall(r"Divider used: (\d+).*?Time: ([\d.e-]+)", block, re.DOTALL)
            for div_id, total_time in matches:
                all_perf_data.append({
                    'Cores': core_val, 
                    'Divider': name_map.get(div_id), 
                    'Time': float(total_time)
                })

    df_raw = pd.DataFrame(all_perf_data)
    df_avg = df_raw.groupby(['Cores', 'Divider'])['Time'].mean().reset_index() if not df_raw.empty else pd.DataFrame()
    
    stats_list = []
    tree_params = "5 1 10 1 10"
    if os.path.exists(stats_file):
        with open(stats_file, 'r', encoding='utf-8') as f:
            stats_content = f.read()
        param_match = re.search(r'generator:?\s*([\d\s]+?)\s+\d{5,}', stats_content)
        if param_match: tree_params = param_match.group(1).strip()
        entries = re.findall(r'experiment number (\d+).*?Seed:\s*(\d+).*?modules:\s*(\d+).*?variables:\s*(\d+)', stats_content, re.DOTALL | re.I)
        for eid, seednum, mods, vars in entries:
            stats_list.append({'SeedID': int(eid), 'SeedNum': int(seednum), 'Modules': int(mods), 'Variables': int(vars)})
    
    df_stats = pd.DataFrame(stats_list)
    return df_avg, df_stats, tree_params

def generate_final_report(output_dir, stats_file, output_excel):
    df_avg, df_stats, tree_params = parse_all_averaged_data(output_dir, stats_file, range(100))
    
    if df_avg.empty or df_stats.empty:
        print("Error: Required data not found.")
        return

    with pd.ExcelWriter(output_excel, engine='xlsxwriter') as writer:
        # 1. Params
        pd.DataFrame({'Parameters': [tree_params]}).to_excel(writer, sheet_name='Summary', startrow=0, index=False)
        
        # 2. Performance Table
        table_main = df_avg.pivot(index='Cores', columns='Divider', values='Time')
        cols = ['Variable', 'Relations', 'Default']
        table_main = table_main.reindex(columns=cols)
        table_main.to_excel(writer, sheet_name='Summary', startrow=3)
        
        # --- NEW: Speedup Calculation ---
        # Get row where Cores == 1 for each divider
        t1 = table_main.loc[1]
        table_speedup = table_main.apply(lambda row: t1 / row, axis=1)
        table_speedup.to_excel(writer, sheet_name='Summary', startrow=3, startcol=5) # Placed next to main table
        # Add a label for the speedup table
        pd.DataFrame(['Speedup Table (T1/Tn)']).to_excel(writer, sheet_name='Summary', startrow=2, startcol=5, index=False, header=False)

        # 3. Stats
        avg_metrics = pd.DataFrame({
            'Avg Modules': [df_stats['Modules'].mean()],
            'Avg Variables': [df_stats['Variables'].mean()]
        })
        avg_metrics.to_excel(writer, sheet_name='Summary', startrow=12, index=False)
        
        # 4. Seeds
        df_stats[['SeedID', 'SeedNum']].to_excel(writer, sheet_name='Summary', startrow=17, index=False)
        
        # 5. Graphs
        worksheet = writer.sheets['Summary']
        
        # --- Graph 1: Performance (Time) ---
        plt.figure(figsize=(6, 4))
        for div in cols:
            subset = df_avg[df_avg['Divider'] == div].sort_values('Cores')
            plt.plot(subset['Cores'], subset['Time'], marker='o', label=div)
        plt.title('Performance Comparison (Execution Time)')
        plt.xlabel('Cores')
        plt.ylabel('Seconds')
        plt.xticks([1, 2, 4, 8, 16])
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.legend(loc='lower center', bbox_to_anchor=(0.5, -0.3), ncol=3)
        
        img_data_time = io.BytesIO()
        plt.savefig(img_data_time, format='png', bbox_inches='tight')
        plt.close()
        worksheet.insert_image('I3', 'graph_time.png', {'image_data': img_data_time})

        # --- Graph 2: Speedup ---
        plt.figure(figsize=(6, 4))
        # Plot ideal speedup line
        # plt.plot([1, 16], [1, 16], color='gray', linestyle='--', label='Ideal Speedup')
        for div in cols:
            plt.plot(table_speedup.index, table_speedup[div], marker='s', label=f'{div} Speedup')
        
        plt.title('Speedup Analysis')
        plt.xlabel('Cores')
        plt.ylabel('Speedup Factor')
        plt.xticks([1, 2, 4, 8, 16])
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.legend(loc='lower center', bbox_to_anchor=(0.5, -0.3), ncol=3)
        
        img_data_speedup = io.BytesIO()
        plt.savefig(img_data_speedup, format='png', bbox_inches='tight')
        plt.close()
        worksheet.insert_image('I25', 'graph_speedup.png', {'image_data': img_data_speedup})

    print(f"Clean report with Speedup created: {output_excel}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python3 time_parser.py <output_dir> <stats_file> <excel_name>")
    else:
        generate_final_report(sys.argv[1], sys.argv[2], sys.argv[3])