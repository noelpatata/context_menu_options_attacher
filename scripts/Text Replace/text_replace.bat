@echo off
REM This batch file launches the Python script for file and content replacement.
REM It receives the folder path from the Windows context menu.

REM Get the path of the directory that was right-clicked on.
REM %~1 removes quotes from the argument, which is useful if the path has spaces.
set "TARGET_FOLDER=%~1"

REM Define the path to your Python script
REM Assuming run_replacer.bat and replace_script.py are in the same folder
set "PYTHON_SCRIPT=%~dp0text_replace.py"

REM Call the Python script, passing the target folder as an argument
python "%PYTHON_SCRIPT%" "%TARGET_FOLDER%"

REM The Python script's 'input("Press Enter...")' will keep the window open.
REM If you remove that line from the Python script, you might want to add 'pause' here.