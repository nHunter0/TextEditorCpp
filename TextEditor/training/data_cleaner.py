# will clean data 
import re

def clean_text(text):
    # Remove Reddit specific blocks
    text = re.sub(r'Upvote\n\d+\n\nDownvote\n\d+\n\ncomments\n\nShare\nShare\n', '', text)
    text = re.sub(r'u\/\w+ avatar\nu\/\w+\n•\s+\d+ days? ago\n', '', text)
    
    # Remove any remaining blocks matching patterns
    text = re.sub(r'\bu\/\w+', '', text)  # Remove usernames
    text = re.sub(r'\d+ days? ago', '', text)  # Remove time stamps
    text = re.sub(r'\d+\n', '', text)  # Remove standalone numbers
    text = re.sub(r'Share\nShare', '', text)  # Remove duplicate share text
    
    # General text cleaning
    text = re.sub(r'[^\w\s]', '', text)  # Remove punctuation
    text = re.sub(r'\s+', ' ', text)  # Replace multiple spaces with a single space
    text = text.strip()  # Remove leading and trailing whitespace
    
    return text

def clean_file(input_file, output_file):
    with open(input_file, 'r', encoding='utf-8') as infile:
        raw_text = infile.read()
        
    cleaned_text = clean_text(raw_text)
    
    with open(output_file, 'w', encoding='utf-8') as outfile:
        outfile.write(cleaned_text)

# Example usage
input_file = 'training_data.txt'
output_file = 'clean_data.txt'
clean_file(input_file, output_file)
