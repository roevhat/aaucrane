% MED CONTAINER
clear all;
close;
run('../model/cranemodel.m');
X = Xm;
W = Wm;
%rlocus(Xm)
C2 = 15;
loop2 = feedback(C2 *  Mxm * gear * (1/s), 1);
%------------------------------------------------------------------------------
% PID Control
%------------------------------------------------------------------------------
% PID = k * ( 1*(b*R(s)-Y(s)) + 1/Ti*(R(s)-Y(s)) + Td*Y(s) );
% I part ->
%bode(loop2*-Wm) %Ti = freq. at -45degrees
Ti = 4.01;
% D part ->
%step(loop2*4) %time below 0.01 amplitude
Td = 0.02; 
% P part ->
Tp = 0.5;     %from formular 0 < Tp <= 1 
%Gain ->
%rlocus((Tp+1+1/Ti*s+Tp*s)*loop2*C2*-Wm)
k = 10;     %Trail and error
%% UDEN CONTAINER
clear all;
close;
run('../model/cranemodel.m');
X = Xu;
W = Wu;
%rlocus(Xu)
C2 = 15;
loop2 = feedback(C2 *  Mxu * gear * (1/s), 1);
%------------------------------------------------------------------------------
% PID Control
%------------------------------------------------------------------------------
% PID = k * ( 1*(b*R(s)-Y(s)) + 1/Ti*(R(s)-Y(s)) + Td*Y(s) );
% I part ->
%bode(loop2*-Wu) %Ti = freq. at -45degrees
Ti = 4.15;
% D part ->
%step(loop2*4) %time below 0.01 amplitude
Td = 0.02; 
% P part ->
Tp = 0.5;     %from formular 0 < Tp <= 1 
%Gain ->
%rlocus((Tp+1+1/Ti*s+Tp*s)*loop2*C2*-Wu)
k = 10;     %Trail and error