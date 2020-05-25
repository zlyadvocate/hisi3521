savepath="/mnt/hd1/chan/"
ext264=".264"
extmp3=".aac"
extavi=".avi"
timeflag=$1
chn0mp3=$savepath"chn0"_$timeflag$extmp3

sleep 5
chn0264=$savepath"chn""0"_$1$ext264
chn0avi=$savepath"chn""0"_$1$extavi

echo $chn0264
echo $chn0avi
/lib/ffmpeg -i $chn0264 -i $chn0mp3 -vcodec copy -acodec copy $chn0avi

sleep 1
rm $chn0264
chn0264=$savepath"chn""1"_$2$ext264
chn0avi=$savepath"chn""1"_$2$extavi
echo $chn0264
echo $chn0avi
/lib/ffmpeg -i $chn0264 -i $chn0mp3 -vcodec copy -acodec copy $chn0avi

sleep 1
rm $chn0264
chn0264=$savepath"chn""2"_$3$ext264
chn0avi=$savepath"chn""2"_$3$extavi
echo $chn0264
echo $chn0avi
/lib/ffmpeg -i $chn0264 -i $chn0mp3 -vcodec copy -acodec copy $chn0avi

sleep 1
rm $chn0264
chn0264=$savepath"chn""3"_$4$ext264
chn0avi=$savepath"chn""3"_$4$extavi
echo $chn0264
echo $chn0avi   
/lib/ffmpeg -i $chn0264 -i $chn0mp3 -vcodec copy -acodec copy $chn0avi
sleep 1

rm $chn0264
#/lib/ffmpeg -i $chn264 -i $chn0mp3 -vcodec copy -acodec copy $chnavi
#sleep 5

echo $chn0mp3
rm $chn0mp3



