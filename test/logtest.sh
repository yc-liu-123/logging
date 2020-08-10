for move in asyn sync; do
	echo "test of" ${move}
	for i in 1 2 3 4 5; do
		./${move}
		sec=`tail -n 1 t_${move}.log | cut -d ' ' -f 4`
		rec=`cat t_${move}.log | wc -l`
		echo "record log num:" ${rec} ", time cost:" ${sec}
		mv t_${move}.log t_${move}_${i}.log
	done
done