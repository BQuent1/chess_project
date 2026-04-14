import trimesh
import sys
import os

def convert(filename, out_filename):
    scene = trimesh.load(filename)
    scene.export(out_filename)
    print(f"Exported {filename} to {out_filename}")

if __name__ == "__main__":
    convert(sys.argv[1], sys.argv[2])
