import os
import sys

def replace_in_files_and_filenames(original_text, text_to_replace, folder_path):
    """
    Replaces occurrences of original_text with text_to_replace in both
    filenames and file contents within the specified folder and its subfolders.

    Args:
        original_text (str): The text to be replaced.
        text_to_replace (str): The text to replace with.
        folder_path (str): The path to the folder to operate on.
    """

    if not os.path.isdir(folder_path):
        print(f"Error: Folder '{folder_path}' not found.")
        return

    for root, dirs, files in os.walk(folder_path, topdown=False):
        # 1. Replace in filenames
        for name in files:
            if original_text in name:
                old_path = os.path.join(root, name)
                new_name = name.replace(original_text, text_to_replace)
                new_path = os.path.join(root, new_name)
                try:
                    os.rename(old_path, new_path)
                    print(f"Renamed: '{old_path}' to '{new_path}'")
                except OSError as e:
                    print(f"Error renaming '{old_path}': {e}")

        # 2. Replace in directory names (after files in that dir are processed)
        for i, name in enumerate(dirs):
            if original_text in name:
                old_path = os.path.join(root, name)
                new_name = name.replace(original_text, text_to_replace)
                new_path = os.path.join(root, new_name)
                try:
                    os.rename(old_path, new_path)
                    print(f"Renamed directory: '{old_path}' to '{new_path}'")
                    # Update dirs list to ensure os.walk continues correctly
                    dirs[i] = new_name
                except OSError as e:
                    print(f"Error renaming directory '{old_path}': {e}")

    # 3. Replace in file contents (after all renames are done to avoid issues with changed paths)
    for root, _, files in os.walk(folder_path):
        for name in files:
            filepath = os.path.join(root, name)
            try:
                # Read the file content
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                # Replace text if found
                if original_text in content:
                    new_content = content.replace(original_text, text_to_replace)
                    # Write the modified content back to the file
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    print(f"Replaced content in: '{filepath}'")
            except Exception as e:
                print(f"Error processing content of '{filepath}': {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        # The first argument (sys.argv[0]) is the script name itself.
        # sys.argv[1] will be the path to the directory that was right-clicked.
        folder_path_input = sys.argv[1]

        print("--- File and Content Replacement Script ---")
        print(f"Operating on: {folder_path_input}\n")

        original_text_input = input("Enter the original text to replace: ")
        text_to_replace_input = input("Enter the text to replace with: ")

        replace_in_files_and_filenames(original_text_input, text_to_replace_input, folder_path_input)
        print("\nOperation complete!")
        input("Press Enter to close the console...") # Keep console open so user can see output
    else:
        print("Error: No folder path provided.")
        print("Usage: This script is intended to be run from the Windows context menu.")
        input("Press Enter to close...")