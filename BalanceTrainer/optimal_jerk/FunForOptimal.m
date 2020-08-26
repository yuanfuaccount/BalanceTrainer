%跃度最优轨迹规划调用
VC=200;
AC=200;
points=[0,0,0,0,0,0;
        15,20,15,4,5,12;
        25,31,42,7,8,6;
        40,60,30,15,14,13;
        50,40,40,8,12,16;
        25,67,20,16,15,14;
        ];
%注意优化时的H上限跟VC有关，因此上限要根据VC和AC的大小具体求解，现在的H上限是以最低速度VC/6运行时获得的。
%当发现加速度超出规定时，降低最低运行速度，如以VC/10运行
[t,pos,speed,Acc,xx]=platformtrajectory(points,VC,AC,0.1,7);

m=size(pos,1);
fid=fopen('C:\Users\15705\Desktop\C++\BalanceTrainer1\trajectorypath.txt','wt');
for i=1:m
    fprintf(fid,'%g\t',pos(i,:));
    fprintf(fid,'\n');
end
fclose(fid);







