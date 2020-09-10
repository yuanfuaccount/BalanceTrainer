namelist=dir('D:\Files\GitRepository\BalanceTrainer\build-Sensor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug\*.csv'); %获取数据列表
num=length(namelist);

Point=300;  %截取500个点
PointStart=1;
PointEnd=PointStart+Point-1;
WX=zeros(num,Point);
WY=zeros(num,Point);
WZ=zeros(num,Point);
AngleX=zeros(num,Point);
AngleY=zeros(num,Point);
AngleZ=zeros(num,Point);
AccX=zeros(num,Point);
AccY=zeros(num,Point);
AccZ=zeros(num,Point);
Type=zeros(num,Point);

for i=1:num
    filename=['D:\Files\GitRepository\BalanceTrainer\build-Sensor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug\',namelist(i).name];
    newData = importdata(filename,',',2);
    raw_data=newData.data;
   % raw_data(any(isnan(raw_data)'),:) = [];
    
    starttime=0;
    delta=1/50;%时间间隔
    endtime=(Point-1)*delta;
    time=[starttime:delta:endtime];
    
     WX(i,:)=raw_data(PointStart:PointEnd,4);
     WY(i,:)=raw_data(PointStart:PointEnd,5);
     WZ(i,:)=raw_data(PointStart:PointEnd,6);
     
    AngleX(i,:)=raw_data(PointStart:PointEnd,7);   
    AngleY(i,:)=raw_data(PointStart:PointEnd,8);
    AngleZ(i,:)=raw_data(PointStart:PointEnd,9);
    
    AccX(i,:)=raw_data(PointStart:PointEnd,1);
    AccY(i,:)=raw_data(PointStart:PointEnd,2);
    AccZ(i,:)=raw_data(PointStart:PointEnd,3);
    
    Type(i,:)=raw_data(PointStart:PointEnd,10);
end

W=zeros(num,Point);
Acc=zeros(num,Point);
for i=1:num
    W(i,:)=sqrt(WX(i,:).^2+WY(i,:).^2+WZ(i,:).^2);
    Acc(i,:)=sqrt(AccX(i,:).^2+AccY(i,:).^2+AccZ(i,:).^2);
end

i=1
while i<=num
    figure();
    subplot(2,1,1);
    plot(time,-WX(i,:));
    hold on;
    plot(time,Acc(i,:)*300-300);
    hold on;
    plotInterval(Type(i,:),0.02);
    hold on;
    plot(time,zeros(1,Point),'r--');
    hold off;
    legend("右角度","右加速度");
    
    subplot(2,1,2);
    plot(time,-WX(i+1,:));
    hold on;
    plot(time,Acc(i+1,:)*300-300);
    hold on;
    plotInterval(Type(i+1,:),0.02);
    hold on;
    plot(time,zeros(1,Point),'r--');
    hold off;
    legend("左角度","左加速度");
    i=i+2;
end










