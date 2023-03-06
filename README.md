# MobileGPS2PC
Transmit mobile phone GPS to server in APOLLO module using websocket. 

## Dependence
[ApolloAuto](https://github.com/ApolloAuto/apollo)


## Usage
1.Compile and run websocket server on Apollo.
```
maingboard -d module/YOURMODULE/websocke_mobile/dag/wesocket.dag
```
2.open getGPS.html on mobile phone browser. (Test on iphone using Edge).
3.click Screen No Sleep ENABLE button to enable screen no sleep.
4.type in local ip address and port, click to connect.
5.enable location authority on phone.
6.bingo

![](https://github.com/Printeger/MobileGPS2PC/raw/master/pic/1.png)
