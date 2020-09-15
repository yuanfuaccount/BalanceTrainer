
function plotInterval(type,deltaT)
%根据type画间隔线
%type:1*n
[m,n]=size(type);
ylim=get(gca,'Ylim');
for i=1:n
    if type(i)==5 %最高点
        x=(i-1)*deltaT;
        plot([x,x],ylim,'m--');
        hold on;
    elseif type(i)==6
        x=(i-1)*deltaT;
        plot([x,x],ylim,'b--');
        hold on;
    elseif type(i)==7
        x=(i-1)*deltaT;
        plot([x,x],ylim,'g--');
        hold on;
    elseif type(i)==8
        x=(i-1)*deltaT;
        plot([x,x],ylim,'r--');
        hold on;
%     elseif type(i)==4 && i+1<=n && i-1>=1 && ((type(i-1)==0 && type(i+1)==4) || (type(i-1)==4 && type(i+1)==0))
%         x=(i-1)*deltaT;
%         plot([x,x],ylim,'b--');
%         hold on;
    end
end
end