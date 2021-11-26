module.exports = {
  "root": true,
  "parser": "@typescript-eslint/parser",
  "parserOptions": {"project": ["./tsconfig.json"]},
  "plugins": [
    "@typescript-eslint"
  ],
  "ignorePatterns": ["node_modules/*", "build/*", "public/pb/*"],
  "env": {
    "jest": true
  },
  "rules": {
    "array-bracket-spacing": ["error", "never"],
    "arrow-spacing": ["error", {"before": true, "after": true}],
    "block-spacing": ["error", "always"],
    "brace-style": ["error", "1tbs", {"allowSingleLine": false}],
    "comma-spacing": ["error", {"before": false, "after": true}],
    "comma-style": ["error", "last"],
    "computed-property-spacing": ["error", "never"],
    "curly": ["error", "all"],
    "dot-location": ["error", "property"],
    "eol-last": ["error"],
    "func-names": ["warn"],
    "indent": ["error", 2, {"SwitchCase": 1}],
    "key-spacing": ["error", {"beforeColon": false, "afterColon": true}],
    "keyword-spacing": ["error"],
    "linebreak-style": ["error", (process.platform === "win32" ? "windows" : "unix")],
    "max-len": ["error", {"code": 140}],
    "no-eq-null": ["off"],
    "no-func-assign": ["error"],
    "no-inline-comments": ["error"],
    "no-mixed-spaces-and-tabs": ["error"],
    "no-multi-spaces": ["error"],
    "no-spaced-func": ["error"],
    "no-trailing-spaces": ["error"],
    "no-var": ["error"],
    "object-curly-spacing": ["error", "always"],
    "one-var": ["error", "never"],
    "one-var-declaration-per-line": ["error"],
    "quotes": ["error", "single"],
    "semi-spacing": ["error", {"before": false, "after": true}],
    "space-before-blocks": ["error"],
    "space-before-function-paren": ["error", {"asyncArrow": "always", "anonymous": "never", "named": "never"}],
    "space-in-parens": ["error", "never"],
    "space-infix-ops": ["error"],
    "space-unary-ops": ["error", {"words": true, "nonwords": false}]
  }
}