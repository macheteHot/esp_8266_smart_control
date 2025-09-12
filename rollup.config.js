import terser from "@rollup/plugin-terser";
import postcss from "rollup-plugin-postcss";
import { minify } from "html-minifier-terser";
import replace from "@rollup/plugin-replace";
import fs from "fs";
import path from "path";
import { gzip } from "zlib";

function getCurrentDate() {
  const now = new Date();
  const year = now.getFullYear();
  const month = String(now.getMonth() + 1).padStart(2, "0"); // 月份从0开始
  const day = String(now.getDate()).padStart(2, "0");
  return `${year}-${month}-${day}`;
}

// 插件：把最终 HTML gzip 后生成 PROGMEM 头文件
function htmlToHeader(outDir, varName = "index_html_gz") {
  return {
    name: "html-to-header",
    async generateBundle(_, bundle) {
      // 获取所有 JS chunk
      const jsContent = Object.values(bundle)
        .filter((f) => f.type === "chunk")
        .map((f) => f.code)
        .join("\n");

      let htmlContent = fs.readFileSync("html/index.html", "utf8");
      htmlContent = htmlContent.replace(
        "</body>",
        `<script>${jsContent}</script>\n</body>`
      );

      const minified = await minify(htmlContent, {
        collapseWhitespace: true,
        removeComments: true,
        minifyCSS: true,
        minifyJS: true,
      });

      const gzipped = await new Promise((resolve, reject) => {
        gzip(minified, { level: 9 }, (err, res) =>
          err ? reject(err) : resolve(res)
        );
      });

      const bytes = Array.from(gzipped).map(
        (b) => `0x${b.toString(16).padStart(2, "0")}`
      );
      const header = `#include <pgmspace.h>
const unsigned char ${varName}[] PROGMEM = {
  ${bytes.join(",")}
};
const unsigned int ${varName}_len = ${gzipped.length};
`;
      fs.mkdirSync(outDir, { recursive: true });
      fs.writeFileSync(path.join(outDir, `${varName}.h`), header);
      console.log(`✅ 生成头文件: ${outDir}/${varName}.h`);
    },
  };
}

export default {
  input: "html/index.js", // JS 入口，可为空
  output: { format: "iife" },
  plugins: [
    replace({
      __BUILD_TIME__: () => getCurrentDate(),
      preventAssignment: true,
    }),
    postcss({
      extract: false, // 内联 CSS
      minimize: true, // 压缩 CSS
    }),
    terser(), // 压缩 JS
    htmlToHeader("./include", "index_html"), // 直接生成头文件
  ],
};
