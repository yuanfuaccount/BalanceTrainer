function H=optimal_jerk(yy,VC,AC,TotalT)  %yy:n*6
n=size(yy,1);

%1.����Ŀ���Ż���������f(H)=(M(i+1)-M(i))^2/H(i)(i=1,2����n-1)��������H,H��n-1ά��
    function f=fun(H)

        %�����綯�׾���A��ͬ���������A
        H0=H(1); H1=H(1);H2=H(2);
        Hn_2=H(n-2); Hn_1=H(n-1);  Hn=H(n-1);
        
        A=zeros(n,n);
        A(1,1)=2+(6/(H0+H1))*(H0^2/(6*H1)+H0/6);  A(1,2)=H1/(H0+H1);
        A(2,1)=(H1/(H1+H2))-(6/(H1+H2))*(H0^2/(6*H1));  A(2,2)=2;  A(2,3)=H2/(H1+H2); 
        A(n-1,n-2)=Hn_2/(Hn_2+Hn_1);  A(n-1,n-1)=2; A(n-1,n)=Hn_1/(Hn_2+Hn_1)-(1/(Hn_2+Hn_1))*(Hn^2/Hn_1); 
        A(n,n-1)=Hn_1/(Hn_1+Hn);  A(n,n)=2+(6/(Hn_1+Hn))*(Hn/6+Hn^2/(6*Hn_1));
        
        %�����D��ά��n*6
        D=zeros(n,6);
        D(1,:)=(6/(H0+H1))*((yy(2,:)-yy(1,:))/H1);
        D(2,:)=(6/(H1+H2))*((yy(3,:)-yy(2,:))/H2-(yy(2,:)-yy(1,:))/H1);    
        D(n-1,:)=6/(Hn_2+Hn_1)*((yy(n,:)-yy(n-1,:))/Hn_1-(yy(n-1,:)-yy(n-2,:))/Hn_2);    
        D(n,:)=(-6/(Hn_1+Hn))*((yy(n,:)-yy(n-1,:))/Hn_1);   
        
        for i=3:n-2
            Hi=H(i);Hi_1=H(i-1);
            A(i,i-1)=Hi_1/(Hi_1+Hi);  A(i,i)=2;  A(i,i+1)=1-A(i,i-1);
            D(i,:)=(6/(Hi_1+Hi))*((yy(i+1,:)-yy(i,:))/Hi-(yy(i,:)-yy(i-1,:))/Hi_1);
        end
          
        M=inv(A)*D;  %n*6
        
        f=0;
        for j=1:6
            for i=1:n-1
                f=f+(M(i+1,j)-M(i,j))^2/H(i);
            end
            f=f+M(1,j)^2/H(1)+M(n,j)^2/H(n-1);
        end
    end

% 2. ��H�ĳ�ʼֵ
%�ҳ�ÿһ�����ֵ
H0=zeros(n-1,1);
H0(1)=max(abs(yy(2,:)-yy(1,:)))/(2*VC);
H0(n-1)=max(abs(yy(n,:)-yy(n-1,:)))/(2*VC);
for i=2:n-2
    H0(i)=max(abs(yy(i+1,:)-yy(i,:)))/VC;
end

% 3.������Լ�������ٶ�Լ���ͼ��ٶ�Լ��,��nonlcon.m


% 4.��ȡhi���½�,��getLB.m
    w=zeros(n-1,1);
    w(1)=max(abs(yy(2,:)-yy(1,:)))/(2*VC);
    w(n-1)=max(abs(yy(n,:)-yy(n-1,:)))/(2*VC);
    for i=2:n-2
        w(i)=max(abs(yy(i+1,:)-yy(i,:)))/VC;
    end
    
% % 5.��ȡhi���Ͻ磬������ȣ�����ȶ���Ϊ��VC/6����ʱ��ʱ��
% u=zeros(n-1,1);
% u(1)=max(abs(yy(2,:)-yy(1,:)))*3/VC;
% u(n-1)=max(abs(yy(n,:)-yy(n-1,:)))*3/VC;
% for i=2:n-2
%     u(i)=max(abs(yy(i+1,:)-yy(i,:)))*6/VC;
% end

%6. ���Ե�ʽԼ��
Aeq=ones(1,n-1);
Aeq(1)=2;
Aeq(n-1)=2;

H=fmincon(@(H)fun(H),H0,[],[],Aeq,TotalT,w,[],@(H)nonlcon(H,yy,VC,AC));

end
