%三次样条曲线轨迹规划
%输入参数： xx:时间序列 yy:时间序列对应的轨迹， deltaT:输出轨迹采样时间间隔
function [time,position,velocity,Acc]=no_optimal_trajectory(xx,yy,deltaT)

n=size(xx,1); %轨迹点个数
t0=xx(1);t2=xx(2);t1=(t2+t0)/2;
tn_2=xx(n-1); tn=xx(n); tn_1=(tn+tn_2)/2;
A=zeros(n,n);M=zeros(n,1);D=zeros(n,6);  %A*M=D，求M

%1.给矩阵A赋值。
h0=t1-t0; h1=t2-t1; h2=xx(3,1)-xx(2,1);    hn_1=tn-tn_1;hn_2=tn_1-tn_2;hn_3=xx(n-1)-xx(n-2);
A(1,1)=2+(6/(h0+h1))*(h0^2/(6*h1)+h0/6);  A(1,2)=h1/(h0+h1); 
A(2,1)=(h1/(h1+h2))-(6/(h1+h2))*(h0^2/(6*h1));  A(2,2)=2;  A(2,3)=h2/(h1+h2); 
A(n-1,n-2)=hn_3/(hn_3+hn_2);  A(n-1,n-1)=2; A(n-1,n)=hn_2/(hn_3+hn_2)-(1/(hn_3+hn_2))*(hn_1^2/hn_2); 
A(n,n-1)=hn_2/(hn_2+hn_1);  A(n,n)=2+(6/(hn_2+hn_1))*(hn_1/6+hn_1^2/(6*hn_2));

D(1,:)=(6/(h0+h1))*((yy(2,:)-yy(1,:))/h1);
D(2,:)=(6/(h1+h2))*((yy(3,:)-yy(2,:))/h2-(yy(2,:)-yy(1,:))/h1);  
D(n-1,:)=6/(hn_3+hn_2)*((yy(n,:)-yy(n-1,:))/hn_2-(yy(n-1,:)-yy(n-2,:))/hn_3);
D(n,:)=(-6/(hn_2+hn_1))*((yy(n,:)-yy(n-1,:))/hn_2);

for i=3:n-2
    hi=xx(i+1)-xx(i);hi_1=xx(i)-xx(i-1);
    A(i,i-1)=hi_1/(hi_1+hi);  A(i,i)=2;  A(i,i+1)=1-A(i,i-1);
    D(i,:)=(6/(hi_1+hi))*((yy(i+1,:)-yy(i,:))/hi-(yy(i,:)-yy(i-1,:))/hi_1);
end

M=inv(A)*D;


q1=yy(1,:)+(h0^2/6)*M(1,:);  qn_1=yy(n,:)+(hn_1^2/6)*M(n,:);
yy_new=zeros(n,6);
yy_new(1,:)=q1; yy_new(n,:)=qn_1; yy_new(2:n-1,:)=yy(2:n-1,:);
xx_new=zeros(n,1);
xx_new(1)=t1;  xx_new(n)=tn_1; xx_new(2:n-1)=xx(2:n-1);

% %画出每一段的轨迹
% figure();
% syms t y;
% 
% Q0=(M(1)/(6*h0))*(t-xx(1))^3+(yy(1)/h0)*(t1-t)+(q1/h0-h0*M(1)/6)*(t-t0);  %第一段函数表达式
% ezplot(y-Q0,[t0,t1],[-180,180]);
% hold on;
% 
% Qn_1=(M(n)/6*hn_1)*(xx(n)-t)^3+(qn_1/hn_1-hn_1*M(n)/6)*(xx(n)-t)+(yy(n)/hn_1)*(t-tn_1); %最后一段函数表达式
% ezplot(y-Qn_1,[tn_1,tn],[-180,180]);
% hold on;
% 
% %中间段函数表达式
% for i=1:n-1
%     hi=xx_new(i+1)-xx_new(i);
%     Q=(M(i)/(6*hi))*(xx_new(i+1)-t)^3+(M(i+1)/(6*hi))*(t-xx_new(i))^3+(yy_new(i)/hi-hi*M(i)/6)*(xx_new(i+1)-t)+(yy_new(i+1)/hi-hi*M(i+1)/6)*(t-xx_new(i));
%     ezplot(y-Q,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% plot(xx_new,yy_new,'r*');
% %hold off;
% title('位移-时间图像');
% xlabel('t(s)');
% ylabel('pos(mm)');
% %pos为最终位移-时间函数
% 
% %画出速度-时间图像
% %figure();
% syms y t;
% V0=(M(1)/(2*h0))*(t-t0)^2+(q1-yy(1))/h0-M(1)*h0/6;
% ezplot(y-V0,[t0,t1],[-180,180]);
% hold on;
% 
% Vn_1=(-M(n)/(2*hn_1))*(tn-t)^2+(yy(n)-qn_1)/hn_1+M(n)*hn_1/6;
% ezplot(y-Vn_1,[tn_1,tn],[-180,180]);
% hold on;
% 
% for i=1:n-1
%     hi=xx_new(i+1)-xx_new(i);
%     Vi=(-M(i)/(2*hi))*(xx_new(i+1)-t)^2+(M(i+1)/(2*hi))*(t-xx_new(i))^2+(yy_new(i+1)-yy_new(i))/hi+(M(i)-M(i+1))*hi/6;
%     ezplot(y-Vi,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% %hold off;
% title('速度-时间图像');
% 
% %加速度，时间图像
% figure();
% syms y t;
% AC0=(t-t0)*M(1)/h0;
% ezplot(y-AC0,[t0,t1],[-180,180]);
% hold on;
% ACn_1=(tn-t)*M(n)/hn_1;
% ezplot(y-ACn_1,[tn_1,tn],[-180,180]);
% hold on;
% for i=1:n-1
%     hi=xx_new(i+1)-xx_new(i);
%     ACi=(xx_new(i+1)-t)*M(i)/hi+(t-xx_new(i))*M(i+1)/hi;
%     ezplot(y-ACi,[xx_new(i),xx_new(i+1)],[-180,180]);
%     hold on;
% end
% hold off;
% title('加速度-时间图像');

%间隔采样，采样时间间隔deltaT,单位s
%res:输出结果m*3矩阵，每列分别为时间，位置，速度
t=[t0:deltaT:tn];
time=t;
m=size(t,2);
position=zeros(m,6);
velocity=zeros(m,6);
Acc=zeros(m,6)
for j=1:6
    pos_j=0;speed_j=0;Acc_j=0;
    pos_j=((M(1,j)/(6*h0))*(t-xx(1)).^3+(yy(1,j)/h0)*(t1-t)+(q1(j)/h0-h0*M(1,j)/6)*(t-t0)).*(t>=t0 & t<t1); 
    speed_j=((M(1,j)/(2*h0))*(t-t0).^2+(q1(j)-yy(1,j))/h0-M(1,j)*h0/6).*(t>=t0 & t<t1);
    Acc_j=(M(1,j)*(t-t0)/h0).*(t>=t0 & t<xx_new(1));

    pos_j=pos_j+((M(n,j)/(6*hn_1))*(xx(n)-t).^3+(qn_1(j)/hn_1-hn_1*M(n,j)/6)*(xx(n)-t)+(yy(n,j)/hn_1)*(t-tn_1)).*(t>=tn_1 & t<=tn);
    speed_j=speed_j+((-M(n,j)/(2*hn_1))*(tn-t).^2+(yy(n,j)-qn_1(j))/hn_1+M(n,j)*hn_1/6).*(t>=tn_1 & t<=tn);
    Acc_j=Acc_j+(M(n,j)*(tn-t)/hn_1).*(t>=tn_1 & t<=tn);

    for i=1:n-1
         hi=xx_new(i+1)-xx_new(i);
         pos_j=pos_j+((M(i,j)/(6*hi))*(xx_new(i+1)-t).^3+(M(i+1,j)/(6*hi))*(t-xx_new(i)).^3+(yy_new(i,j)/hi-hi*M(i,j)/6)*(xx_new(i+1)-t)+(yy_new(i+1,j)/hi-hi*M(i+1,j)/6)*(t-xx_new(i))).*(t>=xx_new(i) & t<xx_new(i+1));
         speed_j=speed_j+((-M(i,j)/(2*hi))*(xx_new(i+1)-t).^2+(M(i+1,j)/(2*hi))*(t-xx_new(i)).^2+(yy_new(i+1,j)-yy_new(i,j))/hi+(M(i,j)-M(i+1,j))*hi/6).*(t>=xx_new(i) & t<xx_new(i+1));
         Acc_j=Acc_j+(M(i,j)*(xx_new(i+1)-t)/hi+M(i+1,j)*(t-xx_new(i))/hi).*(t>=xx_new(i) & t<xx_new(i+1));
    end
    position(:,j)=pos_j;
    velocity(:,j)=speed_j;
    Acc(:,j)=Acc_j;
end

end


