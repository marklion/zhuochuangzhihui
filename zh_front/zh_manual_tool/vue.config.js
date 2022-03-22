module.exports = {
    publicPath: "./",
    pluginOptions: {
        electronBuilder: {
            nodeIntegration: true,
            externals: ['serialport'],
            nodeModulesPath: ['../../node_modules', './node_modules']
        }
    },
}
