#!/bin/bash

# Remove existing folder if it exists
rm -rf aws_ref

# Download tar.gz of data
curl http://www.doc.ic.ac.uk/~txl11/aws_ref_v1.tar.gz -o aws_ref.tar.gz

tar -xzf aws_ref.tar.gz
