var webpack = require('webpack')
var path = require('path')
var HtmlWebpackPlugin = require('html-webpack-plugin')
const UglifyJsPlugin = require('uglifyjs-webpack-plugin')

module.exports = {
  entry: './src/js/entry.js',
  output: {
    path: path.join(__dirname, 'dist'),
    filename: 'bundle.js'
  },
  module: {
    loaders: [
      {
        test: /\.css$/,
        loader: 'style-loader!css-loader'
      }, {
        test: /\.html$/,
        loader: 'raw-loader'
      }, {
        test: /\.(jpe?g|png|woff|woff2|eot|ttf|svg)(\?[a-z0-9=.]+)?$/, 
        loader: 'url-loader?limit=100000'
      }, {
        test: /\.js$/,
        exclude: /(node_modules|bower_components)/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['es2015']
          }
        }
      }
    ]
  },
  plugins: [
    new webpack.BannerPlugin('Add innodisk license here'),
    new webpack.optimize.OccurrenceOrderPlugin(),
    new HtmlWebpackPlugin({
        template: './src/index.html'
    }),
    new webpack.ProvidePlugin({
      "$":"jquery",
      "jQuery":"jquery",
      "window.jQuery":"jquery"
    }),
    new UglifyJsPlugin()
  ]
}