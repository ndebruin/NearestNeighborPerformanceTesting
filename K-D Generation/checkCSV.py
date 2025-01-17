import pandas as pd

def validate_csv(csv_path):
    df = pd.read_csv(csv_path, header=None)
    print(f"Total rows: {len(df)}")
    print(f"Unique rows: {len(df.drop_duplicates())}")
    print(f"Empty rows: {df.isnull().sum().sum()}")


if __name__ == "__main__":
    input_csv = "measurements.csv"
    validate_csv(input_csv)
