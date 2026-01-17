#!/usr/bin/env python3
"""
Huffman Compression Algorithm Implementation

This module implements the Huffman coding algorithm for file compression and decompression.
Huffman coding is a lossless data compression algorithm that uses variable-length codes
for different characters based on their frequency of occurrence.
"""

import heapq
import os
import pickle
from collections import defaultdict, Counter
from typing import Dict, Optional, Tuple


class HuffmanNode:
    """Node in the Huffman tree."""
    
    def __init__(self, char: Optional[str], freq: int, left=None, right=None):
        self.char = char
        self.freq = freq
        self.left = left
        self.right = right
    
    def __lt__(self, other):
        """Compare nodes based on frequency for heap operations."""
        return self.freq < other.freq


class HuffmanCompressor:
    """Huffman compression and decompression implementation."""
    
    def __init__(self):
        self.codes: Dict[str, str] = {}
        self.reverse_codes: Dict[str, str] = {}
    
    def _calculate_frequencies(self, data: bytes) -> Dict[int, int]:
        """Calculate frequency of each byte in the data."""
        return Counter(data)
    
    def _build_huffman_tree(self, frequencies: Dict[int, int]) -> Optional[HuffmanNode]:
        """Build Huffman tree from character frequencies."""
        if not frequencies:
            return None
        
        # Create a min heap of nodes
        heap = []
        for char, freq in frequencies.items():
            node = HuffmanNode(char, freq)
            heapq.heappush(heap, node)
        
        # Build the tree
        while len(heap) > 1:
            left = heapq.heappop(heap)
            right = heapq.heappop(heap)
            
            # Create internal node with combined frequency
            merged = HuffmanNode(None, left.freq + right.freq, left, right)
            heapq.heappush(heap, merged)
        
        return heap[0] if heap else None
    
    def _generate_codes(self, root: Optional[HuffmanNode], current_code: str = ""):
        """Generate Huffman codes for each character by traversing the tree."""
        if root is None:
            return
        
        # Leaf node - contains a character
        if root.char is not None:
            # Handle the special case of single character file
            self.codes[root.char] = current_code if current_code else "0"
            return
        
        # Traverse left (0) and right (1)
        self._generate_codes(root.left, current_code + "0")
        self._generate_codes(root.right, current_code + "1")
    
    def _encode_data(self, data: bytes) -> str:
        """Encode data using Huffman codes."""
        encoded = []
        for byte in data:
            encoded.append(self.codes[byte])
        return ''.join(encoded)
    
    def _decode_data(self, encoded_data: str, root: HuffmanNode) -> bytes:
        """Decode Huffman encoded data back to original bytes."""
        if not encoded_data:
            return b''
        
        decoded = []
        current_node = root
        
        # Handle single character case
        if root.char is not None:
            count = len(encoded_data)
            decoded = [root.char] * count
        else:
            for bit in encoded_data:
                # Traverse tree based on bit
                if bit == '0':
                    current_node = current_node.left
                else:
                    current_node = current_node.right
                
                # Reached leaf node
                if current_node.char is not None:
                    decoded.append(current_node.char)
                    current_node = root
        
        return bytes(decoded)
    
    def compress_file(self, input_path: str, output_path: str) -> Tuple[int, int, float]:
        """
        Compress a file using Huffman coding.
        
        Args:
            input_path: Path to input file
            output_path: Path to output compressed file
        
        Returns:
            Tuple of (original_size, compressed_size, compression_ratio)
        """
        # Read input file
        with open(input_path, 'rb') as f:
            data = f.read()
        
        if not data:
            # Handle empty file
            with open(output_path, 'wb') as f:
                pickle.dump({'tree': None, 'encoded': '', 'padding': 0}, f)
            return 0, 0, 0.0
        
        original_size = len(data)
        
        # Calculate frequencies
        frequencies = self._calculate_frequencies(data)
        
        # Build Huffman tree
        root = self._build_huffman_tree(frequencies)
        
        # Generate codes
        self.codes = {}
        self._generate_codes(root)
        
        # Encode data
        encoded_data = self._encode_data(data)
        
        # Convert binary string to bytes
        # Add padding to make it a multiple of 8
        padding = 8 - len(encoded_data) % 8
        if padding != 8:
            encoded_data += '0' * padding
        else:
            padding = 0
        
        # Convert to bytes
        encoded_bytes = bytearray()
        for i in range(0, len(encoded_data), 8):
            byte = encoded_data[i:i+8]
            encoded_bytes.append(int(byte, 2))
        
        # Save compressed data with tree and padding info
        compressed_data = {
            'tree': root,
            'encoded': bytes(encoded_bytes),
            'padding': padding
        }
        
        with open(output_path, 'wb') as f:
            pickle.dump(compressed_data, f)
        
        compressed_size = os.path.getsize(output_path)
        compression_ratio = (1 - compressed_size / original_size) * 100 if original_size > 0 else 0
        
        return original_size, compressed_size, compression_ratio
    
    def decompress_file(self, input_path: str, output_path: str) -> int:
        """
        Decompress a Huffman encoded file.
        
        Args:
            input_path: Path to compressed file
            output_path: Path to output decompressed file
        
        Returns:
            Size of decompressed file
        """
        # Load compressed data
        with open(input_path, 'rb') as f:
            compressed_data = pickle.load(f)
        
        root = compressed_data['tree']
        encoded_bytes = compressed_data['encoded']
        padding = compressed_data['padding']
        
        if root is None:
            # Empty file
            with open(output_path, 'wb') as f:
                f.write(b'')
            return 0
        
        # Convert bytes back to binary string
        encoded_data = ''
        for byte in encoded_bytes:
            encoded_data += format(byte, '08b')
        
        # Remove padding
        if padding:
            encoded_data = encoded_data[:-padding]
        
        # Decode data
        decoded_data = self._decode_data(encoded_data, root)
        
        # Write decompressed data
        with open(output_path, 'wb') as f:
            f.write(decoded_data)
        
        return len(decoded_data)


def main():
    """Main function for command-line usage."""
    import sys
    
    if len(sys.argv) < 4:
        print("Usage:")
        print("  Compress:   python huffman.py compress <input_file> <output_file>")
        print("  Decompress: python huffman.py decompress <input_file> <output_file>")
        sys.exit(1)
    
    mode = sys.argv[1]
    input_file = sys.argv[2]
    output_file = sys.argv[3]
    
    compressor = HuffmanCompressor()
    
    if mode == 'compress':
        if not os.path.exists(input_file):
            print(f"Error: Input file '{input_file}' not found")
            sys.exit(1)
        
        print(f"Compressing '{input_file}'...")
        original_size, compressed_size, ratio = compressor.compress_file(input_file, output_file)
        print(f"Compression complete!")
        print(f"Original size: {original_size} bytes")
        print(f"Compressed size: {compressed_size} bytes")
        print(f"Compression ratio: {ratio:.2f}%")
    
    elif mode == 'decompress':
        if not os.path.exists(input_file):
            print(f"Error: Input file '{input_file}' not found")
            sys.exit(1)
        
        print(f"Decompressing '{input_file}'...")
        decompressed_size = compressor.decompress_file(input_file, output_file)
        print(f"Decompression complete!")
        print(f"Decompressed size: {decompressed_size} bytes")
    
    else:
        print(f"Error: Unknown mode '{mode}'. Use 'compress' or 'decompress'")
        sys.exit(1)


if __name__ == '__main__':
    main()
