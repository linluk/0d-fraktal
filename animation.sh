
zoom=1.0
directory=anim
files=
rm -rf $directory
mkdir $directory
for i in `seq 1 200`;
do
  file=$(printf "$directory/%06d.ppm" $i)
  files=$(echo $files $file)
  ./0d-fraktal -d 800x800 -c -1.502+0i -b 200 -i 2048 -t 4 -z $zoom -o $file
  zoom=$(echo $zoom*1.1 | bc --quiet)
done
convert -delay 100 -loop 0 files animation.gif
