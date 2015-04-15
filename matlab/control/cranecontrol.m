%==============================================================================
% AAUCrane Control (C1 (X), C2 (W), C3 (Y))
%==============================================================================
% Load models (Mx, My, X, Y, W)
run('../model/cranemodel.m');

%------------------------------------------------------------------------------
% X(s) Control
%------------------------------------------------------------------------------
% rlocus(X) => No overshoot + Max gain => 26.3
C1 = 11.6;
% Close the loop
innerLoop = feedback(C1*X, 1);

%------------------------------------------------------------------------------
% W(s) Control
%------------------------------------------------------------------------------
% rlocus(-innerLoop*W) shows that proportional control is not good enough!
% The minimum overshoot obtainable is ~45% => Sucks...
C2 = 1.8;

%------------------------------------------------------------------------------
% Y(s) Control
%------------------------------------------------------------------------------
% rlocus(Y) => No overshoot + Max gain => 49.3
C3 = 100;