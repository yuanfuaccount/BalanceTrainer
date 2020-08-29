%跃度最优轨迹规划调用
VC=200;
AC=200;
points=[0,0,0,0,0,0;
        15,20,35,15,14,26;
        65,94,99,8,8,12;
        120,150,189,4,14,13;
        250,70,246,8,12,16;
        100,191,136,16,15,14;
        ];
%注意优化时的H上限跟VC有关，因此上限要根据VC和AC的大小具体求解，现在的H上限是以最低速度VC/6运行时获得的。
%当发现加速度超出规定时，降低最低运行速度，如以VC/10运行
[t,pos,speed,Acc,xx]=platformtrajectory(points,VC,AC,0.1,8);
No obvious advantageNo obvious advantage
m=size(pos,1);
fid=fopen('C:\Users\15705\Desktop\C++\BalanceTrainer1\trajectorypath.txt','wt');
for i=1:m
    fprintf(fid,'%g\t',pos(i,:));
    fprintf(fid,'\n');
end
fclose(fid);







