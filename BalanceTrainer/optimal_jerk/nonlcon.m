function [g,h]=nonlcon(H,yy,VC,AC)
        n=size(yy,1);
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

        b=zeros(n,1);
        b(:)=AC;
        v=zeros(n-1,1);
        v(:)=VC;
        
        H0=H(1);   Hn_1=H(n-1);  Hn=Hn_1;
        q1=yy(1,:)+(H0^2/6)*M(1,:);  qn=yy(n,:)+(Hn_1^2/6)*M(n,:);  %中间两个虚拟点坐标

        yy_new=zeros(n,6);
        yy_new(1,:)=q1;  yy_new(n,:)=qn;  yy_new(2:n-1,:)=yy(2:n-1,:);
        V=zeros(n-1,1);
        for i=1:n-1
            Vi_a=-H(i)*M(i,:).*M(i+1,:)./(2*(M(i+1,:)-M(i,:)))+(yy_new(i+1,:)-yy_new(i,:))/H(i)-H(i)*(M(i+1,:)-M(i,:))/6;
            Vi_b=-M(i,:)*H(i)/2+(yy_new(i+1,:)-yy_new(i,:))/H(i)+(M(i,:)-M(i+1,:))*H(i)/6;
            Vi_c=M(i+1,:)*H(i)/2+(yy_new(i+1,:)-yy_new(i,:))/H(i)+(M(i,:)-M(i+1,:))*H(i)/6;
            Vmatrix=[Vi_a;Vi_b;Vi_c];
            V(i)=max(max(abs(Vmatrix)));
        end
        Mmax=max(abs(M),[],2) %每一行最大值
        g=[Mmax-b;
           V-v
           ]; %速度，加速度约束
        h=[];

    end