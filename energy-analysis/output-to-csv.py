import re
import sys
import csv

def parse_output(output):
    # Split the output into lines
    lines = output.strip().split('\n')

    # Extract the header line
    header_line = lines[0]
    # Extract column names from the header line
    column_names = [col.strip() for col in re.split(r'\s*\|\s*|\s*x\s*', header_line) if col.strip()]

    # Initialize a list to store the parsed data
    data = []

    # Iterate over the data lines
    for line in lines[2:]:  # Skip the header and separator lines
        # Split the line into columns
        columns = re.split(r'\s*\|\s*', line.strip())
        columns = columns[1:-1]

        # Check if the number of columns matches the header
        if len(columns) != len(column_names):
            print(f"Skipping malformed line: {line}")
            continue

        # Create a dictionary for each row
        row_dict = {}
        for i, col in enumerate(columns):
            col = col.strip()
            if col:
                row_dict[column_names[i]] = col
        # Add the row dictionary to the data list
        data.append(row_dict)

    return data

def read_file_and_parse(file_path):
    try:
        with open(file_path, 'r') as file:
            output = file.read()
        return parse_output(output)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return []

def export_to_csv(data, output_file):
    if not data:
        print("No data to export.")
        return

    # Extract the header from the first dictionary
    header = data[0].keys()

    # Write the data to a CSV file
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=header)
        writer.writeheader()
        writer.writerows(data)
    print(f"Data exported to {output_file}")

def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_csv_file>")
        return

    input_file = sys.argv[1]
    output_csv_file = sys.argv[2]

    parsed_data = read_file_and_parse(input_file)
    export_to_csv(parsed_data, output_csv_file)

if __name__ == "__main__":
    main()

