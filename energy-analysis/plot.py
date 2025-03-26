import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def plot_gflop_per_watt(csv_files):
    # Define markers and colors for each dataset
    markers = ['o', 's', '^']  # Circle, square, triangle
    colors = ['b', 'g', 'r']   # Blue, green, red

    # Create a plot
    plt.figure(figsize=(10, 6))

    # Iterate over the CSV files and plot each one
    for i, csv_file in enumerate(csv_files):
        # Read the CSV file into a DataFrame
        df = pd.read_csv(csv_file)

        # Convert 'ID' to numeric, ensuring it's treated as a continuous variable
        df['ID'] = pd.to_numeric(df['ID'], errors='coerce')

        # Convert 'GFLOP/watt' to numeric
        df['GFLOP/watt'] = pd.to_numeric(df['GFLOP/watt'], errors='coerce')

        # Drop rows with NaN values in 'ID' or 'GFLOP/watt'
        df = df.dropna(subset=['ID', 'GFLOP/watt'])

        # Sample one point out of every 20 IDs
        sampled_df = df[df.index % 1 == 0]

        # Find the index of the maximum GFLOP/watt value in the sampled data
        max_index = sampled_df['GFLOP/watt'].idxmax()
        max_point = sampled_df.loc[max_index]

        # Plot the sampled data points with the specified marker and color
        plt.scatter(sampled_df['ID'], sampled_df['GFLOP/watt'], marker=markers[i], color=colors[i], label=os.path.basename(csv_file))

        # Label the point with the highest GFLOP/watt in the sampled data
        plt.annotate(f'{max_point["GFLOP/watt"]} GFLOP/watt for ID={max_point["ID"]}',
                     (max_point['ID'], max_point['GFLOP/watt']),
                     textcoords="offset points",
                     xytext=(5,5),
                     ha='center',
                     fontsize=14,
                     color=colors[i])

    # Add title and labels
    plt.title('GFLOP/watt vs ID (Sampled)')
    plt.xlabel('ID')
    plt.ylabel('GFLOP/watt')
    plt.grid(True)
    plt.legend()
    plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: python script.py <csv_file1> <csv_file2> ... <csv_file_n>")
        return

    csv_files = sys.argv[1::]
    plot_gflop_per_watt(csv_files)

if __name__ == "__main__":
    main()

