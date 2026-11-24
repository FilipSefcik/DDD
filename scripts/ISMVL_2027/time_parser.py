import glob
import io
import os
import re
import sys

import matplotlib
matplotlib.use('Agg')  # no display needed, we only save images
import matplotlib.pyplot as plt
import pandas as pd

TIME_RE = re.compile(r'Time:\s*([-+\d.eE]+)')
SECTION_RE = r'=={name}==(.*?)(?:==[A-Z]+==|\Z)'


def section_time(content, name):
    """Time printed by DDD/main inside the ==NAME== block (last one if there are more)."""
    m = re.search(SECTION_RE.format(name=name), content, re.DOTALL)
    if not m:
        return None
    times = TIME_RE.findall(m.group(1))
    return float(times[-1]) if times else None


def parse_times(output_dir):
    rows = []
    for path in glob.glob(os.path.join(output_dir, 'script_output_divider_*.txt')):
        exp_id = int(re.search(r'_(\d+)\.txt$', path).group(1))
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
        rows.append({
            'Experiment': exp_id,
            'Decomposed': section_time(content, 'DECOMPOSED'),
            'Merged': section_time(content, 'MERGED'),
        })
    if not rows:
        return pd.DataFrame(columns=['Experiment', 'Decomposed', 'Merged'])
    return pd.DataFrame(rows).sort_values('Experiment').reset_index(drop=True)


def parse_stats(stats_file):
    tree_params = ''
    entries_list = []
    if os.path.exists(stats_file):
        with open(stats_file, 'r', encoding='utf-8') as f:
            content = f.read()
        m = re.search(r'generator:?\s*([\d\s]+?)\s+\d{5,}', content)
        if m:
            tree_params = m.group(1).strip()
        entries = re.findall(
            r'experiment number (\d+).*?Seed:\s*(\d+).*?modules:\s*(\d+).*?variables:\s*(\d+)',
            content, re.DOTALL | re.I)
        for eid, seed, mods, vars_ in entries:
            entries_list.append({'Experiment': int(eid), 'Seed': int(seed),
                                 'Modules': int(mods), 'Variables': int(vars_)})
    df = pd.DataFrame(entries_list, columns=['Experiment', 'Seed', 'Modules', 'Variables'])
    return df, tree_params


def make_bar_chart(mean_dec, mean_merged, n):
    fig, ax = plt.subplots(figsize=(5, 4))
    labels = ['Decomposed', 'Merged']
    values = [mean_dec, mean_merged]
    bars = ax.bar(labels, values, color=['#4C72B0', '#DD8452'], width=0.5)
    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(), f'{val:.4g} s',
                ha='center', va='bottom')
    ax.set_ylabel('Computation time [s]')
    ax.set_title(f'Average computation time ({n} experiments)')
    ax.grid(axis='y', linestyle='--', alpha=0.6)
    ax.set_axisbelow(True)
    ax.set_ylim(0, max(values) * 1.15)

    buf = io.BytesIO()
    fig.savefig(buf, format='png', bbox_inches='tight')
    plt.close(fig)
    return buf


def generate_final_report(output_dir, stats_file, output_excel):
    df_times = parse_times(output_dir)
    df_stats, tree_params = parse_stats(stats_file)

    if df_times.empty:
        print("Error: no experiment outputs found.")
        return

    df = df_times.merge(df_stats, how='left', on='Experiment')
    df = df[['Experiment', 'Seed', 'Modules', 'Variables', 'Decomposed', 'Merged']]

    valid = df.dropna(subset=['Decomposed', 'Merged'])
    missing = len(df) - len(valid)
    if missing:
        print(f"Warning: {missing} experiment(s) have a missing time and are excluded from averages.")
    if valid.empty:
        print("Error: no experiment has both a decomposed and a merged time.")
        return

    mean_dec, mean_merged = valid['Decomposed'].mean(), valid['Merged'].mean()

    avg_row = {'Experiment': 'Average', 'Seed': '',
               'Modules': valid['Modules'].mean(), 'Variables': valid['Variables'].mean(),
               'Decomposed': mean_dec, 'Merged': mean_merged}
    df_out = pd.concat([df, pd.DataFrame([avg_row])], ignore_index=True)

    # CSV with every experiment + the average row at the end
    csv_path = os.path.splitext(output_excel)[0] + '.csv'
    df_out.to_csv(csv_path, index=False)

    summary = pd.DataFrame({
        'Metric': ['Mean [s]', 'Std dev [s]', 'Min [s]', 'Max [s]'],
        'Decomposed': [mean_dec, valid['Decomposed'].std(), valid['Decomposed'].min(),
                       valid['Decomposed'].max()],
        'Merged': [mean_merged, valid['Merged'].std(), valid['Merged'].min(),
                   valid['Merged'].max()],
    })

    with pd.ExcelWriter(output_excel, engine='xlsxwriter') as writer:
        df_out.to_excel(writer, sheet_name='Results', index=False)

        ws = writer.book.add_worksheet('Summary')
        writer.sheets['Summary'] = ws
        ws.write(0, 0, 'Generator parameters (depth min_sons max_sons min_slots max_slots states)')
        ws.write(1, 0, tree_params)
        ws.write(3, 0, f'Experiments used: {len(valid)}')
        summary.to_excel(writer, sheet_name='Summary', startrow=5, index=False)
        ws.write(11, 0, 'Speedup (Decomposed / Merged), >1 means merged is faster')
        ws.write(11, 1, mean_dec / mean_merged)
        ws.insert_image('E2', 'average_times.png',
                        {'image_data': make_bar_chart(mean_dec, mean_merged, len(valid))})

    print(f"Report created: {output_excel} (+ {csv_path})")
    print(f"Mean decomposed: {mean_dec:.6g} s | mean merged: {mean_merged:.6g} s "
          f"| decomposed/merged: {mean_dec / mean_merged:.3f}")


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python3 time_parser.py <output_dir> <stats_file> <excel_name>")
    else:
        generate_final_report(sys.argv[1], sys.argv[2], sys.argv[3])