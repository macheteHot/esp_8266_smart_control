import subprocess
import sys

try:
    subprocess.run(["npx", "rollup", "-c", "--silent"], check=True)
    print("✅ Rollup 构建完成")
except subprocess.CalledProcessError as e:
    print("❌ Rollup 构建失败", file=sys.stderr)
    sys.exit(1)
