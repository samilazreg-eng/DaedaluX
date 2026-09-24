import csv
import argparse
import re

# This script converts a CSV file to Daikon .dtrace and .decls files
# The CSV file must have a header row with the variable names
# The CSV file must have the same number of values in each row
# The CSV file must have the same number of values in each column


import re

def format_var_name(input):
    """
    Formats a variable name by replacing spaces, double quotes, slashes, brackets, and angle brackets with underscores,
    and quoting backslashes and quotes (in that order).

    Args:
        input (str): The input string to format.

    Returns:
        str: The formatted variable name.
    """
    simplified = re.sub(r'[\s\"\/\(\)\[\]{}<>-]', '_', input.strip())
    simplified = simplified.replace('\\', '\\\\').replace('"', '\\"')
    return '"' + simplified + '"'


def is_number(s):
    """
    Returns True if the input string can be converted to a float, False otherwise.

    Args:
        s (str): The input string to check.

    Returns:
        bool: True if the input string can be converted to a float, False otherwise.
    """
    try:
        float(s)
        return True
    except ValueError:
        return False
    
def parse_decl(inputfile):
    try:
        with open(inputfile, 'r') as declhandle:
            declare = declhandle.readline().strip()
            if declare != "DECLARE":
                raise ValueError("First line of the declaration file must be 'DECLARE'")
            
            ppt = declhandle.readline().strip()
            varnames = []
            varDecType = []
            varActType = []
            varComparable = []
            for line in declhandle:
                varname = line.strip()
                varDecType.append(declhandle.readline().strip())
                varActType.append(declhandle.readline().strip())
                varComparable.append(declhandle.readline().strip())
                varnames.append(varname)
            return ppt, varnames, varDecType, varActType, varComparable
    except FileNotFoundError:
        raise ValueError(f"Cannot open declarations file {inputfile}")


def interpolate(values):
    """
    Interpolates missing values in a list of strings.

    Args:
        values (list): A list of strings.

    Returns:
        list: A list of strings with missing values interpolated.
    """
    interpolated = []
    last_valid_value = None
    for value in values:
        if value.strip() == "":
            if last_valid_value is not None:
                interpolated.append(last_valid_value)
            else:
                interpolated.append("0" if is_number(value) else '""')
        else:
            interpolated.append(value)
            last_valid_value = value
    return interpolated

def main():
    parser = argparse.ArgumentParser(description='Convert CSV to Daikon .dtrace and .decls files')
    parser.add_argument('input_filename', help='Input CSV filename', type=str)
    parser.add_argument('-m', '--missing-action', choices=['nonsensical', 'old', 'interpolate', 'zero'], default='old', help='Behavior for missing values')
    parser.add_argument('-d', '--declarations', help='Declaration file name')

    print(f'CSV to Daikon converter script')
    
    args = parser.parse_args()

    with open(args.input_filename, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        csv_varnames = next(csv_reader)
        csv_varnames = [format_var_name(varname) for varname in csv_varnames]
        print(f'CSV file has {len(csv_varnames)} variables: {csv_varnames}')
        num_decl_vars = len(csv_varnames)

        prevvalues = []
        isNumber = []
        varNameCsvIndex = {}

        for i in range(len(csv_varnames)):
            prevvalues.append(0)  # Initialize all previous values to 0
            isNumber.append(1)    # Initialize all types to number
            varNameCsvIndex[csv_varnames[i]] = i
        

        dtrace_filename = args.input_filename.replace('.csv', '.dtrace')
        program_point_name = 'aprogram.point:::POINT'


        with open(dtrace_filename, 'w') as dtrace_file:
            for row in csv_reader:
                if len(row) != num_decl_vars:
                    print(f'Warning: Row does not match the number of variables ({num_decl_vars}), skipping.')
                    continue
                # Strip whitespace from all values
                row = [value.strip() for value in row]

                dtrace_file.write(f'{program_point_name}\n')
                for header, value in zip(csv_varnames, row):
                    if value.strip() == "": # Missing value or empty string
                        if args.missing_action == 'old': # Use the previous value
                            value = prev_values.get(header)
                        elif args.missing_action == 'zero': # Use 0 or "" depending on the type
                            value = "0" if is_number(prev_values.get(header)) else '""'
                        elif args.missing_action == 'nonsensical': # Use a nonsensical value
                            value = 'nonsensical'
                        elif args.missing_action == 'interpolate': # Interpolate between the previous and next values
                            interpolated = interpolate(variables.get(header, []))
                            value = interpolated.pop(0)
                            variables[header] = interpolated
                    else:
                        if is_number(value):
                            value = value
                            isNumber[varNameCsvIndex[header]] = 1
                        else:
                            value = f'"{value.strip()}"'
                            isNumber[varNameCsvIndex[header]] = 0
                        prev_values[header] = value

                    # Write the variable name and value to the dtrace file
                    dtrace_file.write(f'{header}\n')
                    dtrace_file.write(f'{value}\n')
                    dtrace_file.write('1\n') # Always 1

                # Add a newline between each program point
                dtrace_file.write('\n')



        decls_file = args.input_filename.replace('.csv', '.decls')
        with open(decls_file, 'w') as decls_handle:
            decls_handle.write(f'DECLARE\n{program_point_name}\n')
            for j in range(num_decl_vars):
                csvindex = varNameCsvIndex[csv_varnames[j]]
                decls_handle.write(f'{csv_varnames[csvindex]}\n')
                # Add the type of the variable to the decls file - string or double
                if isNumber[csvindex]:
                    decls_handle.write('double\ndouble\n')
                else:
                    decls_handle.write('java.lang.String\njava.lang.String\n')
                decls_handle.write('1\n')

    print(f'CSV to Daikon conversion completed successfully.')

if __name__ == "__main__":
    prev_values = {}
    variables = {}
    main()