%轨迹规划函数,输入yy维度n*6,每根电动缸的轨迹点
%输入参数： yy:通过的轨迹点  VC:速度上限  AC:加速度上限  delatT:时间采样间隔
%输出参数： time:时间序列  pos:位置序列  speed:速度序列  Acc:加速度序列 xx:输入位置序列对应的事件序列
function [time,pos,speed,Acc,xx]=trajectory(yy,VC,AC,deltaT,TotalT)

n=size(yy,1); %轨迹点个数
H=optimal_jerk(yy,VC,AC,TotalT);

%求M
A=zeros(n,n);
D=zeros(n,6);
H0=H(1); H1=H(1); H2=H(2);
Hn_2=H(n-2); Hn_1=H(n-1);  Hn=H(n-1);

A(1,1)=2+(6/(H0+H1))*(H0^2/(6*H1)+H0/6);  A(1,2)=H1/(H0+H1);  
A(2,1)=(H1/(H1+H2))-(6/(H1+H2))*(H0^2/(6*H1));  A(2,2)=2;  A(2,3)=H2/(H1+H2); 
A(n-1,n-2)=Hn_2/(Hn_2+Hn_1);  A(n-1,n-1)=2; A(n-1,n)=Hn_1/(Hn_2+Hn_1)-(1/(Hn_2+Hn_1))*(Hn^2/Hn_1); 
A(n,n-1)=Hn_1/(Hn_1+Hn);  A(n,n)=2+(6/(Hn_1+Hn))*(Hn/6+Hn^2/(6*Hn_1));

D(1,:)=(6/(H0+H1))*((yy(2,:)-yy(1,:))/H1);
D(2,:)=(6/(H1+H2))*((yy(3,:)-yy(2,:))/H2-(yy(2,:)-yy(1,:))/H1);
D(n-1,:)=6/(Hn_2+Hn_1)*((yy(n,:)-yy(n-1,:))/Hn_1-(yy(n-1,:)-yy(n-2,:))/Hn_2);
D(n,:)=(-6/(Hn_1+Hn))*((yy(n,:)-yy(n-1,:))/Hn_1);

for i=3:n-2
    Hi=H(i);Hi_1=H(i-1);
    A(i,i-1)=Hi_1/(Hi_1+Hi);  A(i,i)=2;  A(i,i+1)=1-A(i,i-1);
    D(i,:)=(6/(Hi_1+Hi))*((yy(i+1,:)-yy(i,:))/Hi-(yy(i,:)-yy(i-1,:))/Hi_1);
end

M=inv(A)*D; %n*6

h0=H(1);  hn_1=H(n-1);
q1=yy(1,:)+(h0^2/6)*M(1,:);  qn=yy(n,:)+(hn_1^2/6)*M(n,:);
yy_new=zeros(n,6);
yy_new(1,:)=q1; yy_new(n,:)=qn; yy_new(2:n-1,:)=yy(2:n-1,:);
xx_new=zeros(n,1);
xx_new(1)=H(1);
for i=2:n
    xx_new(i)=xx_new(i-1)+H(i-1);
end

%输入位置序列对应的时间序列
xx=zeros(n,1);
xx(1)=0;  xx(n)=xx_new(n)+H(i-1);
xx(2:n-1)=xx_new(2:n-1);

t0=0; t1=xx_new(1);
hn=H(n-1); tn_p1=xx_new(n)+H(n-1);

% %画出每一段的轨迹
% figure();
% syms t y;
%  %第一段函数表达式
% Q0=(M(1)/(6*h0))*(t-t0)^3+(yy(1)/h0)*(t1-t)+(q1/h0-h0*M(1)/6)*(t-t0); 
% ezplot(y-Q0,[t0,t1],[-180,180]);
% hold on;
% 
% 
% Qn=(M(n)/(6*hn))*(tn_p1-t)^3+(qn/hn-hn*M(n)/6)*(tn_p1-t)+(yy(n)/hn)*(t-xx_new(n)); %最后一段函数表达式
% ezplot(y-Qn,[xx_new(n),tn_p1],[-180,180]);
% hold on;
% 
% 
% %中间段函数表达式
% for i=1:n-1
%     hi=H(i);
%     Qi=(M(i)/(6*hi))*(xx_new(i+1)-t)^3+(M(i+1)/(6*hi))*(t-xx_new(i))^3+(yy_new(i)/hi-hi*M(i)/6)*(xx_new(i+1)-t)+(yy_new(i+1)/hi-hi*M(i+1)/6)*(t-xx_new(i));
%     ezplot(y-Qi,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% 
% plot(xx_new,yy_new,'r*');
% title('位移-时间图像');
% xlabel('t(s)');
% ylabel('pos(mm)');
% 
% 
% %速度-时间图像
% figure();
% syms y t;
% V0=(M(1)/(2*h0))*(t-t0)^2+(q1-yy(1))/h0-M(1)*h0/6;
% ezplot(y-V0,[t0,t1],[-180,180]);
% hold on;
% 
% Vn=(-M(n)/(2*hn))*(tn_p1-t)^2+(yy(n)-qn)/hn+M(n)*hn/6;
% ezplot(y-Vn,[xx_new(n),tn_p1],[-180,180]);
% hold on;
% 
% for i=1:n-1
%     hi=H(i);
%     Vi=(-M(i)/(2*hi))*(xx_new(i+1)-t)^2+(M(i+1)/(2*hi))*(t-xx_new(i))^2+(yy_new(i+1)-yy_new(i))/hi+(M(i)-M(i+1))*hi/6;
%     ezplot(y-Vi,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% 
% title('速度-时间图像');
% 
% 
% %加速度，时间图像
% figure();
% syms y t;
% AC0=(t-t0)*M(1)/h0;
% ezplot(y-AC0,[t0,t1],[-180,180]);
% hold on;
% 
% ACn=(tn_p1-t)*M(n)/hn;
% ezplot(y-ACn,[xx_new(n),tn_p1],[-180,180]);
% hold on;
% 
% for i=1:n-1
%     hi=H(i);
%     ACi=(xx_new(i+1)-t)*M(i)/hi+(t-xx_new(i))*M(i+1)/hi;
%     ezplot(y-ACi,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% title('加速度-时间图像');
% hold off;

%间隔采样，采样时间间隔deltaT,单位s
%res:输出结果m*3矩阵，每列分别为时间，位置，速度

t=[t0:deltaT:tn_p1];
time=t;
m=size(t,2);
pos=zeros(m,6);
speed=zeros(m,6);
Acc=zeros(m,6);

for j=1:6
    pos_j=0; speed_j=0; Acc_j=0;
    pos_j=((M(1,j)/(6*h0))*(t-t0).^3+(yy(1,j)/h0)*(t1-t)+(q1(j)/h0-h0*M(1,j)/6)*(t-t0)).*(t>=t0 & t<xx_new(1)); 
    speed_j=((M(1,j)/(2*h0))*(t-t0).^2+(q1(j)-yy(1,j))/h0-M(1,j)*h0/6).*(t>=t0 & t<xx_new(1));
    Acc_j=(M(1,j)*(t-t0)/h0).*(t>=t0 & t<xx_new(1));
    pos_j=pos_j+((M(n,j)/(6*hn))*(tn_p1-t).^3+(qn(j)/hn-hn*M(n,j)/6)*(tn_p1-t)+(yy(n,j)/hn)*(t-xx_new(n))).*(t>=xx_new(n) & t<=tn_p1);
    speed_j=speed_j+((-M(n,j)/(2*hn))*(tn_p1-t).^2+(yy(n,j)-qn(j))/hn+M(n,j)*hn/6).*(t>=xx_new(n) & t<=tn_p1);
    Acc_j=Acc_j+(M(n,j)*(tn_p1-t)/hn).*(t>xx_new(n) & t<=tn_p1);
    for i=1:n-1
        hi=H(i);
        pos_j=pos_j+((M(i,j)/(6*hi))*(xx_new(i+1)-t).^3+(M(i+1,j)/(6*hi))*(t-xx_new(i)).^3+(yy_new(i,j)/hi-hi*M(i,j)/6)*(xx_new(i+1)-t)+(yy_new(i+1,j)/hi-hi*M(i+1,j)/6)*(t-xx_new(i))).*(t>=xx_new(i) & t<xx_new(i+1));
        speed_j=speed_j+((-M(i,j)/(2*hi))*(xx_new(i+1)-t).^2+(M(i+1,j)/(2*hi))*(t-xx_new(i)).^2+(yy_new(i+1,j)-yy_new(i,j))/hi+(M(i,j)-M(i+1,j))*hi/6).*(t>=xx_new(i) & t<xx_new(i+1));
        Acc_j=Acc_j+((xx_new(i+1)-t)*M(i,j)/hi+(t-xx_new(i))*M(i+1,j)/hi).*(t>=xx_new(i) & t<xx_new(i+1));
    end
    pos(:,j)=pos_j';
    speed(:,j)=speed_j';
    Acc(:,j)=Acc_j';
end







end

