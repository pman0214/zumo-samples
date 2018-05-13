# Zumo開発サンプル

## 概要

無線LANの設定やMilkcocoaの基本設定は ``truestudio/camtank/app_config.h`` に施す。
アプリのソースは ``truestudio/camtank/`` 内に置いた.cppファイルに書く。
.cfgファイルもあわせて用意すること。

必要に応じてMakefile（.mkファイル）や.hファイルを用意する。
.cfgファイルの名前はMakefileで指定できる。
Makefileでは自分が使用するライブラリをincludeする。

TrueSTUDIOを使ってビルドする場合は、各Makefileの `USE_TRUESTUDIO` を `true` に変更すること。

## サンプル

``truestudio/camtank/app.cpp`` に入っている。

* ``app.cpp`` Zumoを動かすサンプル。
* ``mqtt_pubsub_sample.cpp`` MQTTの使い方サンプル。無線LAN接続も含む。
* ``accel.cpp`` 加速度センサの読み取りサンプル（周期ハンドラを使用）。
* ``milkcocoa_coordinate.cpp`` Milkcocoaを使ったZumo 2台の連携サンプル。

タスクや周期ハンドラの操作に関してはuITRONの仕様書やTOPPERSの仕様書などを参照すること。

* http://www.ertl.jp/ITRON/SPEC/mitron4-j.html
* http://www.toppers.jp/documents.html#ngki_spec
