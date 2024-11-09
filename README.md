## Step 1: find cases whose fixing commit only changes one function.
```
python filter.py
```
After this step, 1522 valid cases are stored in `cases.json`.

## Step 2: find cases whose security and functionality can be evaluated.
### Step 2.1: find compilable and testable cases.
The top 40 compilable and testable projects and their corresponding cases can be found in `top_40.json`. These cases are also stored in `ids/all_ids.csv`.

### Step 2.2: Execute triggering inputs and unit tests.
```
bash run_setup.sh ids/all_ids.csv
bash run.sh ids/all_ids.csv ids/all_ids.log
python analyze_report.py --step execution
```
The 428 valid cases are stored in `ids/testable_ids.csv`.

### Step 2.3: Insert print statement into vulnerable function and execute the unit tests.
```
python insert_print.py
bash run.sh ids/testable_ids.csv ids/testable_ids.csv
python analyze_report.py --step print_check
```
The 61 valid cases are stored in `ids/print_checked_ids.csv`.

## Step 3: Create mask and run coverage tests.
### Step 3.1: Create mask.
```
python mask.py
```

### Step 3.2: Run coverage tests.
```
bash run.sh ids/print_checked_ids.csv ids/print_checked_ids.log
python analyze_report.py --step coverage
```
The valid cases are stored in `ids/covered_ids.csv`.

### Step 3.3: Manually review masks and rerun coverage tests.
Manual work. The ??? valid cases are stored in `ids/final_ids.csv`.

## Step 4: Local variable perturbation.
### Step 4.1: Randomly select a variable before the mask.
```
python extract_var.py
```

### Step 4.2: Replace the variable name with a new one.
```
python replace_var.py
```

## Step 5: Get description.
### Step 5.1: Use GPT-4o to get descriptions.
```
python get_desc.py
```

### Step 5.2: Manual work.
See `descriptions/{id}/desc.txt`