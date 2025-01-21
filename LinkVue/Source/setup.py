import os

def convert_cpp_to_txt(directory):
    # Create a "Text" folder in the specified directory if it doesn't exist
    text_folder = os.path.join(directory, "Text")
    os.makedirs(text_folder, exist_ok=True)
    
    # Iterate through files in the given directory
    for filename in os.listdir(directory):
        # Process only .cpp and .h files
        if filename.endswith(".cpp") or filename.endswith(".h"):
            # Read the content of the current file
            file_path = os.path.join(directory, filename)
            with open(file_path, "r", encoding="utf-8") as file:
                content = file.read()
            
            # Create a unique .txt filename for each file
            base_name = os.path.splitext(filename)[0]
            extension = os.path.splitext(filename)[1].lstrip(".")  # Get extension without the dot
            txt_filename = f"{base_name}_{extension}.txt"
            txt_path = os.path.join(text_folder, txt_filename)
            
            # Write content to the .txt file
            with open(txt_path, "w", encoding="utf-8") as txt_file:
                txt_file.write(content)
            
            print(f"Converted: {filename} -> {txt_filename}")

# Replace with the path to your directory
directory_path = "./"
convert_cpp_to_txt(directory_path)
