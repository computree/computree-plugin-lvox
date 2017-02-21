set terminal pngcairo
set output 'trust.png'
set title 'Trust factor according to the number of blocked rays'
set xlabel 'Blocked rays'
set ylabel 'Trust factor'
set grid

plot 'trust.data' with linespoints
