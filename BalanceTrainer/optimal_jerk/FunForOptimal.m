%Ծ�����Ź켣�滮����
VC=200;
AC=200;
points=[0,0,0,0,0,0;
        15,20,15,4,5,12;
        25,31,42,7,8,6;
        40,60,30,15,14,13;
        50,40,40,8,12,16;
        25,67,20,16,15,14;
        ];
%ע���Ż�ʱ��H���޸�VC�йأ��������Ҫ����VC��AC�Ĵ�С������⣬���ڵ�H������������ٶ�VC/6����ʱ��õġ�
%�����ּ��ٶȳ����涨ʱ��������������ٶȣ�����VC/10����
[t,pos,speed,Acc,xx]=platformtrajectory(points,VC,AC,0.1,7);

m=size(pos,1);
fid=fopen('C:\Users\15705\Desktop\C++\BalanceTrainer1\trajectorypath.txt','wt');
for i=1:m
    fprintf(fid,'%g\t',pos(i,:));
    fprintf(fid,'\n');
end
fclose(fid);







