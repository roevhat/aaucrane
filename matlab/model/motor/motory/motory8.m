%%
CRANE_URL = 'http://172.26.12.144/data';
TMP_FILE  = 'tmp.csv';

websave(TMP_FILE, [CRANE_URL '/crane/ysteps/1429011236.csv']);

DATA = csvread(TMP_FILE, 2, 0);
t = DATA(:,1) * 1e-6;

idx_end = find(t > 10);
idx_end = idx_end(1);

t = t(1:idx_end);

t = linspace(0,t(idx_end), idx_end);

y = DATA(:,5) * -.1536 + 1.2106;
y = y(1:idx_end) - y(1);
u = DATA(:,9);
u = (-u(1:idx_end)) + u(idx_end);

save measmotory8 t u y

process = 'motory8'

delete(TMP_FILE);

% Senstool params: pare = [.0028 .0013]
