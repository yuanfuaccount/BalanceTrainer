%ƽ̨Ծ�����Ź켣�滮������������ռ�����е���ά����,�ٶ����ޣ����ٶ����ޣ�ʱ����
%points:n*6�Ķ�ά����ÿһ�д���ÿ�������ά����
%��ά�����Ӧ˳��x,y,z,r,p,y
%deltaT:����ʱ�䣬һ��ȡ100ms
%TotalT:������ʱ��
function [time,pos,speed,Acc,xx]=platformtrajectory(points,VC,AC,deltaT,TotalT) 
%ƽ̨�Ļ�������
P1_0=[213.23,249.33,0];P2_0=[109.31,309.33,0];P3_0=[-322.54,60,0];
P4_0=[-322.54,-60,0];P5_0=[109.31,-309.33,0];P6_0=[213.23,-249.33,0]; %��ƽ̨����
P0=[P1_0;P2_0;P3_0;P4_0;P5_0;P6_0];

B1_0=[652.56,60,0];B2_0=[-274.32,595.14,0];B3_0=[-378.24,535.14,0];
B4_0=[-378.24,-535.14,0];B5_0=[-274.32,-595.14,0];B6_0=[652.56,-60,0]; %��ƽ̨����
B0=[B1_0;B2_0;B3_0;B4_0;B5_0;B6_0];

T0=[0;0;450]; %��ʼ�߶�

%������綯�׵�λ�����
n=size(points,1);
yy=zeros(n,6); %�����綯�׶�Ӧ����
for i=1:n
    eul=[points(i,6)*pi/180,points(i,5)*pi/180,points(i,4)*pi/180]; %Ĭ��˳��Z,Y,X����Y,P,R
    R=eul2rotm(eul); %��ת����
    T=T0+[points(i,1);points(i,2);points(i,3)]; %ƽ�ƾ���
    P=R*P0'+T; %��ƽ̨������ƽ̨������
    L=P-B0'; %ÿ���綯�׵ĳ�������, 3*6
    for j=1:6
        yy(i,j)=sqrt(L(:,j)'*L(:,j));
    end
end

%���TotalT>=��С����ʱ��
yy_minus=zeros(n-1,6);
for i=1:n-1
    yy_minus(i,:)=abs(yy(i+1,:)-yy(i,:));
end
yy_minus_min=min(yy_minus,[],2);
T=sum(yy_minus_min)/VC;
if TotalT<T
    error("Total run time is too shprt");
end

%yy:n*6,��Ӧÿ���綯�׵Ĺ켣��
[time,pos,speed,Acc,xx]=trajectory(yy,VC,AC,deltaT,TotalT);

%�����켣�滮ͼ
paint=1; %Ĭ�ϻ�ͼ
if paint
    color=['k--','b--','g--','r--','m--','y--'];
    
    figure()
    for i=1:6
        plot(time,pos(:,i),color(i));
        hold on;
    end
    legend("���1","���2","���3","���4","���5","���6");
    
    for i=1:6
        plot(xx,yy(:,i),'k*');
        hold on;
    end
    hold off;
    title("λ��-ʱ��ͼ��");
    xlabel("s");
    ylabel("mm");
    
    figure();
    for i=1:6
        plot(time,speed(:,i),color(i));
        hold on;
    end
    hold off;
    legend("���1","���2","���3","���4","���5","���6");
    title("�ٶ�-ʱ��ͼ��");
    xlabel("s");
    ylabel("mm/s");
    
    figure();
    for i=1:6
        plot(time,Acc(:,i),color(i));
        hold on;
    end
    hold off;
    legend("���1","���2","���3","���4","���5","���6");
    title("���ٶ�-ʱ��ͼ��");
    xlabel("s");
    ylabel("mm/s^2");
end

contrast=1; %Ĭ������£�����Ծ���������Ծ�����ŵĶԱ�
if contrast
    xx1=[0;1.6;3.2;4.8;6.4;8];
    [time1,pos1,speed1,Acc1]=no_optimal_trajectory(xx1,yy,0.1);
end
if paint&&contrast
    color=['k-','b-','g-','r-','m-','y-'];
    color1=['k..','b..','g..','r..','m..','y..'];
    figure();
    for j=1:3
        plot(time,pos(:,j),color(j));
        hold on;
        plot(time1,pos1(:,j),color1(j));
        hold on;
    end
    hold off;
    title("�켣�Ա�");
    legend("Ծ���Ż��綯��1�켣","���Ż��綯��1�켣","Ծ���Ż��綯��2�켣","���Ż��綯��2�켣","Ծ���Ż��綯��3�켣","���Ż��綯��3�켣");
    
    figure();
    for j=1:3
        plot(time,speed(:,j),color(j));
        hold on;
        plot(time1,speed1(:,j),color1(j));
        hold on;
    end
    hold off;
    title("�ٶȶԱ�");
    legend("Ծ���Ż��綯��1�ٶ�","���Ż��綯��1�ٶ�","Ծ���Ż��綯��2�ٶ�","���Ż��綯��2�ٶ�","Ծ���Ż��綯��3�ٶ�","���Ż��綯��3�ٶ�");
    
    figure();
    for j=1:3
        plot(time,Acc(:,j),color(j));
        hold on;
        plot(time1,Acc1(:,j),color1(j));
        hold on;
    end
    hold off;
    title("���ٶȶԱ�");
    legend("Ծ���Ż��綯��1���ٶ�","���Ż��綯��1���ٶ�","Ծ���Ż��綯��2���ٶ�","���Ż��綯��2���ٶ�","Ծ���Ż��綯��3���ٶ�","���Ż��綯��3���ٶ�");
    
    %���Ӧ��Ծ��
    optimalJerk=zeros(n-1,6);
    Jerk=zeros(n-1,6);
    for i=2:n
        t2=round(xx(i)/deltaT);
        t1=round(xx(i-1)/deltaT);
        if t1==0
            optimalJerk(i-1,:)=Acc(t2,:).^2/(xx(i)-xx(i-1));
        else
            optimalJerk(i-1,:)=(Acc(t2,:)-Acc(t1,:)).^2/(xx(i)-xx(i-1));
        end
        tt2=round(xx1(i)/deltaT);
        tt1=round(xx1(i-1)/deltaT);
        if tt1==0
            Jerk(i-1,:)=Acc1(tt2,:).^2/(xx1(i)-xx1(i-1));
        else
            Jerk(i-1,:)=(Acc1(tt2,:)-Acc1(tt1,:)).^2/(xx1(i)-xx1(i-1));
        end
        
    end
    opjerk=sum(optimalJerk,2);
    nonopjerk=sum(Jerk,2);
    figure();
    xx_new=xx(1:end-1);
    stairs(xx_new,opjerk);
    hold on;
    xx1_new=xx1(1:end-1);
    stairs(xx1_new,nonopjerk);
    title("Ծ��");
    legend("���Ź켣�滮","��ͨ�켣�滮");
end

end