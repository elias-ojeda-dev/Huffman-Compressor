# Huffman Compressor

A Python implementation of the Huffman coding algorithm for lossless file compression and decompression.

## What is Huffman Coding?

Huffman coding is a popular lossless data compression algorithm that assigns variable-length codes to characters based on their frequency of occurrence. Characters that appear more frequently are assigned shorter codes, while less frequent characters get longer codes, resulting in overall compression.

## Features

- **Lossless Compression**: Original file can be perfectly reconstructed
- **Efficient Algorithm**: Uses optimal prefix-free coding
- **Universal**: Works with any file type (text, binary, etc.)
- **Command-line Interface**: Easy to use from terminal

## Installation

This implementation requires Python 3.6 or higher. No external dependencies are needed as it uses only Python standard library.

```bash
git clone https://github.com/XxCarlosOjeda12/Hoffman-Compressor.git
cd Hoffman-Compressor
```

## Usage

### Compress a File

```bash
python huffman.py compress <input_file> <output_file>
```

Example:
```bash
python huffman.py compress document.txt document.huff
```

### Decompress a File

```bash
python huffman.py decompress <input_file> <output_file>
```

Example:
```bash
python huffman.py decompress document.huff document_restored.txt
```

## How It Works

1. **Frequency Analysis**: The algorithm analyzes the input file and calculates the frequency of each byte.

2. **Tree Building**: A binary tree is constructed where:
   - Each leaf node represents a character
   - Each internal node represents the sum of frequencies of its children
   - The tree is built bottom-up using a priority queue (min-heap)

3. **Code Generation**: Traversing the tree from root to each leaf generates unique binary codes:
   - Left edges are labeled '0'
   - Right edges are labeled '1'
   - Each character's code is the path from root to its leaf

4. **Encoding**: The input file is encoded by replacing each byte with its corresponding Huffman code.

5. **Decoding**: The encoded data is decoded by traversing the Huffman tree based on the bits in the compressed file.

## Example

Create a sample text file and compress it:

```bash
# Create a sample file
echo "this is an example of a huffman tree" > sample.txt

# Compress the file
python huffman.py compress sample.txt sample.huff

# Decompress the file
python huffman.py decompress sample.huff sample_restored.txt

# Verify they are identical
diff sample.txt sample_restored.txt
```

## File Format

The compressed file contains:
- The Huffman tree structure (for decoding)
- The encoded data as bytes
- Padding information (to handle bit-level encoding)

These components are serialized using Python's pickle format.

## Limitations

- The compressed file includes the Huffman tree, which adds overhead. This means very small files might actually increase in size after compression.
- Best compression ratios are achieved with larger files that have non-uniform byte distributions.
- The pickle format is Python-specific, so decompression requires Python.

## Performance

Compression efficiency depends on the input file:
- **Text files**: Typically 40-60% compression for English text
- **Already compressed files** (ZIP, JPEG, etc.): Little to no compression (may increase size)
- **Repetitive data**: Can achieve very high compression ratios

## License

This project is open source and available for educational purposes.

## Contributing

Feel free to open issues or submit pull requests with improvements!