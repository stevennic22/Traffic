// webpack.common.js

const path = require("path");

module.exports = {
  entry: {
    app: "./src/App.jsx",
  },
  resolve: {
    extensions: [".", ".js", ".jsx"],
  },
  output: {
    filename: "[name].js",
    path: path.resolve(__dirname, "dist"),
    clean: true,
  },
  optimization: {
    runtimeChunk: false,
  },
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        loader: "babel-loader",
      },
      {
        test: /\.(svg|ico|png|jpg|jpeg|gif|webp)$/,
        use: {
          loader: "file-loader",
          options: {
            name: "[name].[ext]",
            outputPath: "assets/img",
          },
        },
      },
      {
        test: /\.(otf|eot|ttf|woff2|woff)$/,
        use: {
          loader: "file-loader",
          options: {
            name: "[name].[ext]",
            outputPath: "assets/font",
          },
        },
      },
    ],
  },
};
