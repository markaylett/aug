xy <- read.table(file='stdin')
png(filename='bench.png')
plot(xy, col='blue', main='echo server', pch=20, xlab='elapsed/sec', ylab='roundtrip/ms')
dev.off()
