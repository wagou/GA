概要
GAアルゴリズムを用いて巡回セールスマン問題を解くC言語のプログラム。
ga.c……標準GA
ga_b.c…ボールドウィン型GA
ga_l.c…ラマルク型GA
ga_test.c……標準GA(10回連続実行)
gab_test.c…ボールドウィン型GA(10回連続実行)
gal_test.c…ラマルク型GA(10回連続実行)
#########################
動作環境
Windows 10 version 10.0.18362.535 + gcc version 6.3.0
#########################
コンパイル例
$ gcc -Wall -o ga ga.c -lm
#########################
実行方法
単体で実行する場合、都市数は20となる。
$ ./ga
都市数を設定したい場合、都市数を付け加えて実行する。(例:都市数50)
$ ./ga 50