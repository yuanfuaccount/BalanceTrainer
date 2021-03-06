%平台跃度最优轨迹规划，输入参数，空间点序列的三维坐标,速度上限，加速度上限，时间间隔
%points:n*6的二维矩阵，每一行代表每个点的六维坐标
%六维坐标对应顺序x,y,z,r,p,y
%deltaT:采样时间，一般取100ms
%TotalT:运行总时间
function [time,pos,speed,Acc,xx]=platformtrajectory(points,VC,AC,deltaT,TotalT) 
%平台的基本参数
P1_0=[213.23,249.33,0];P2_0=[109.31,309.33,0];P3_0=[-322.54,60,0];
P4_0=[-322.54,-60,0];P5_0=[109.31,-309.33,0];P6_0=[213.23,-249.33,0]; %上平台坐标
P0=[P1_0;P2_0;P3_0;P4_0;P5_0;P6_0];

B1_0=[652.56,60,0];B2_0=[-274.32,595.14,0];B3_0=[-378.24,535.14,0];
B4_0=[-378.24,-535.14,0];B5_0=[-274.32,-595.14,0];B6_0=[652.56,-60,0]; %下平台坐标
B0=[B1_0;B2_0;B3_0;B4_0;B5_0;B6_0];

T0=[0;0;450]; %初始高度

%求各个电动缸的位置逆解
n=size(points,1);
yy=zeros(n,6); %六根电动缸对应长度
for i=1:n
    eul=[points(i,6)*pi/180,points(i,5)*pi/180,points(i,4)*pi/180]; %默认顺序Z,Y,X，即Y,P,R
    R=eul2rotm(eul); %旋转矩阵
    T=T0+[points(i,1);points(i,2);points(i,3)]; %平移矩阵
    P=R*P0'+T; %上平台点在下平台的坐标
    L=P-B0'; %每根电动缸的长度向量, 3*6
    for j=1:6
        yy(i,j)=sqrt(L(:,j)'*L(:,j));
    end
end

%检查TotalT>=最小运行时间
yy_minus=zeros(n-1,6);
for i=1:n-1
    yy_minus(i,:)=abs(yy(i+1,:)-yy(i,:));
end
yy_minus_min=min(yy_minus,[],2);
T=sum(yy_minus_min)/VC;
if TotalT<T
    error("Total run time is too shprt");
end

%yy:n*6,对应每根电动缸的轨迹点
[time,pos,speed,Acc,xx]=trajectory(yy,VC,AC,deltaT,TotalT);

%画出轨迹规划图
paint=1; %默认画图
% if paint
%     color=['k--','b--','g--','r--','m--','y--'];
%     
%     figure()
%     for i=1:6
%         plot(time,pos(:,i),color(i));
%         hold on;
%     end
%     legend("电缸1","电缸2","电缸3","电缸4","电缸5","电缸6");
%     
%     for i=1:6
%         plot(xx,yy(:,i),'k*');
%         hold on;
%     end
%     hold off;
%     title("位置-时间图像");
%     xlabel("s");
%     ylabel("mm");
%     
%     figure();
%     for i=1:6
%         plot(time,speed(:,i),color(i));
%         hold on;
%     end
%     hold off;
%     legend("电缸1","电缸2","电缸3","电缸4","电缸5","电缸6");
%     title("速度-时间图像");
%     xlabel("s");
%     ylabel("mm/s");
%     
%     figure();
%     for i=1:6
%         plot(time,Acc(:,i),color(i));
%         hold on;
%     end
%     hold off;
%     legend("电缸1","电缸2","电缸3","电缸4","电缸5","电缸6");
%     title("加速度-时间图像");
%     xlabel("s");
%     ylabel("mm/s^2");
% end

contrast=1; %默认情况下，进行跃度最优与非跃度最优的对比
if contrast
    xx1=[0;1.6;3.2;4.8;6.4;8];
    [time1,pos1,speed1,Acc1]=no_optimal_trajectory(xx1,yy,0.1);
end
if paint&&contrast
    color=['k-','b-','g-','r-','m-','y-'];
    color1=['k..','b..','g..','r..','m..','y..'];
    %位置对比
    for j=1:6
        figure();
        plot(time,pos(:,j),'b-','LineWidth',1.4);
        hold on;
        plot(time1,pos1(:,j),'b--','LineWidth',1.4);
        hold on;
        plot(xx,yy(:,j),'r*','LineWidth',1.4);
        hold on;
        plot(xx1,yy(:,j),'ro','LineWidth',1.4);
        hold off;
        legend("optimal","ordinary");
        title({['trajectory of joint ',num2str(j)]});
        xlabel("time(s)");
        ylabel("pos(mm)");
        set(gca,'FontSize',18);
        set(gca,'LineWidth',1.4);
        set(get(gca,'XLabel'),'FontSize',18);
        set(get(gca,'YLabel'),'FontSize',18);
        set(get(gca,'title'),'FontSize',18);
        set(get(gca,'legend'),'FontSize',18);
        str=sprintf( 'pos%d', j);
        %print('-djpeg',str); 
    end
    %速度对比
    for j=1:6
        figure();
        plot(time,speed(:,j),'b-','LineWidth',1.4);
        hold on;
        plot(time1,speed1(:,j),'b--','LineWidth',1.4);
        hold off;
        legend("optimal","ordinary");
        title({['velocity of joint ',num2str(j)]});
        xlabel("time(s)");
        ylabel("velocity(mm/s)");
        set(gca,'FontSize',15);
        set(gca,'LineWidth',1.4);
        set(get(gca,'XLabel'),'FontSize',15);
        set(get(gca,'YLabel'),'FontSize',15);
        set(get(gca,'title'),'FontSize',15);
        set(get(gca,'legend'),'FontSize',15);
        str=sprintf( 'vec%d', j);
        %print('-djpeg',str); 
    end
    %加速度对比
    
    for j=1:6
        figure();
        plot(time,Acc(:,j),'b-','LineWidth',1.4);
        hold on;
        plot(time1,Acc1(:,j),'b--','LineWidth',1.4);
        hold off;
        legend("optimal","ordinary");
        title({['accleration of joint ',num2str(j)]});
        xlabel("time(s)");
        ylabel("acceleration(mm/s^2)");
        set(gca,'FontSize',15);
        set(gca,'LineWidth',1.4);
        set(get(gca,'XLabel'),'FontSize',15);
        set(get(gca,'YLabel'),'FontSize',15);
        set(get(gca,'title'),'FontSize',15);
        set(get(gca,'legend'),'FontSize',15);
        str=sprintf( 'acc%d', j);
        %print('-djpeg',str); 
    end
    
    %求对应的跃度
    m1=size(time,2);
    optimalJerk=zeros(m1-1,6);
    Jerk=zeros(m1-1,6);
    for i=2:m1
        optimalJerk(i-1,:)=(Acc(i,:)-Acc(i-1,:)).^2/(time(i)-time(i-1));
        Jerk(i-1,:)=(Acc1(i,:)-Acc1(i-1,:)).^2/(time(i)-time(i-1));
    end
    opjerk=sum(optimalJerk,2);
    nonopjerk=sum(Jerk,2);
    figure();
    stairs(time(1:m1-1),opjerk,'b-','LineWidth',1.4);
    hold on;
    stairs(time(1:m1-1),nonopjerk,'b--','LineWidth',1.4);
    a=sum(opjerk);
    b=sum(nonopjerk);
    legend("optimal","ordinary");
    set(gca,'FontSize',15);
    set(gca,'LineWidth',1.4);
    set(get(gca,'XLabel'),'FontSize',15);
    set(get(gca,'YLabel'),'FontSize',15);
    set(get(gca,'title'),'FontSize',15);
    set(get(gca,'legend'),'FontSize',15);
    str=sprintf( 'jerk%d', j);
    %print('-djpeg',str); 
end

end