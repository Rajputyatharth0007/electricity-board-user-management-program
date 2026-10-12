$prog = "C:\Users\user\Desktop\coding\c coding\electricity.exe"
$dir  = "C:\Users\user\Desktop\coding\c coding"

function Run-Test {
    param([int]$num, [string]$label)
    Write-Host ""
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host ("TEST {0}: {1}" -f $num, $label) -ForegroundColor Cyan
    Write-Host "==========================================" -ForegroundColor Cyan
    $inputFile = Join-Path $dir ("t{0}.txt" -f $num)
    $inputData = Get-Content $inputFile -Raw
    $pinfo = New-Object System.Diagnostics.ProcessStartInfo
    $pinfo.FileName = $prog
    $pinfo.RedirectStandardInput  = $true
    $pinfo.RedirectStandardOutput = $true
    $pinfo.RedirectStandardError  = $true
    $pinfo.UseShellExecute = $false
    $p = New-Object System.Diagnostics.Process
    $p.StartInfo = $pinfo
    $null = $p.Start()
    $p.StandardInput.Write($inputData)
    $p.StandardInput.Close()
    $out = $p.StandardOutput.ReadToEnd()
    $err = $p.StandardError.ReadToEnd()
    $p.WaitForExit()
    Write-Host $out
    if ($err) { Write-Host $err -ForegroundColor Red }
}

# Clear data files
Remove-Item (Join-Path $dir "users.txt")    -ErrorAction SilentlyContinue
Remove-Item (Join-Path $dir "requests.txt") -ErrorAction SilentlyContinue
Write-Host ">> Cleared users.txt and requests.txt for fresh test run" -ForegroundColor Yellow

Run-Test 1  "View users + requests on EMPTY data (no files)"
Run-Test 2  "Add User 1: Yatharth Rajput, mobile 9876543210"
Run-Test 3  "Add User 2: Priya Sharma (spaces in name), mobile 9123456789"
Run-Test 4  "Reject invalid mobile (12345 - too short)"
Run-Test 5  "Reject duplicate mobile 9876543210"
Run-Test 6  "View all registered users"
Run-Test 7  "Add connection request for User 1 (by mobile)"
Run-Test 8  "Add connection request for User 2 (by User ID)"
Run-Test 9  "Reject request for non-existent user 9999999999"
Run-Test 10 "View all connection requests"
Run-Test 11 "Edit request 1: keep address, set status to Approved"
Run-Test 12 "Edit request 2: new address and set status to Rejected"
Run-Test 13 "Edit non-existent request ID 999"
Run-Test 14 "Invalid menu choices (abc and 9)"
Run-Test 15 "Final view - data persistence after all edits"

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "FINAL DATA FILE CONTENTS" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host "--- users.txt ---"
$uf = Join-Path $dir "users.txt"
if (Test-Path $uf) { Get-Content $uf } else { Write-Host "(not found)" }
Write-Host "--- requests.txt ---"
$rf = Join-Path $dir "requests.txt"
if (Test-Path $rf) { Get-Content $rf } else { Write-Host "(not found)" }
Write-Host ""
Write-Host "ALL TESTS COMPLETE." -ForegroundColor Green
