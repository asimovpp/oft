# coding: utf-8
import sys
import re
import pandas as pd

rx_dict = {"id_line": re.compile(r"ID (\d+) given to ├  (.+) on Line (\d+) in file (.+)")}


def _parse_line(line, rx_dict):
    for key, rx in rx_dict.items():
        match = rx.search(line)
        if match:
            return key, match

    return None, None


def parse_file(filename):
    data = []

    with open(filename, "r") as f:
        for line in f:
            #print(line)
            key, m = _parse_line(line, rx_dict)
            if key == "id_line":
                instr_id = int(m.group(1))
                instr = m.group(2)
                line_num = int(m.group(3))
                instr_file = m.group(4)
                datum = {"instr_id": instr_id, "instr": instr, "line_num": line_num, "instr_file": instr_file}
                data.append(datum)

    return data


def print_for_comparison(df):
    for instrf in df.instr_file.drop_duplicates().sort_values():
        print(instrf)
        subdf = df.loc[df.instr_file == instrf]
        print(subdf.line_num.sort_values())


if __name__ == "__main__":
    if len(sys.argv) > 2:
        print("Handling of multiple input files not implemented. Exiting.")
        exit(1)

    all_data = []
    for filename in sys.argv[1:]:
        all_data = all_data + parse_file(filename)

    df = pd.DataFrame(all_data)
    print_for_comparison(df)
