m = 5;  %서버 수 
K = 6;   %큐의 크기
M = 20; %전체 시스템 Customer 수

range = 1:9; 
rho = range./10; %utilization factor : 0.1 ~0.9

syms k
Series1 = symsum(rho.^k.*nchoosek(M,k),k,0,m-1);  %symsum:  (1/10)^k*MCk ,k가 0~m-1 까지 일때 급수 합
Series2 = symsum(rho.^k.*nchoosek(M,k).*factorial(k)./factorial(m)*(m^(m-k)),k,m,K);
prob_0 = 1./(Series1+Series2);  %P0 확률

prob_k = zeros(9,6);  %P1 ~ P6 확률 저장
for i=1:1:9
    for j=1:1:m-1
        prob_k(i,j) = prob_0(i) * rho(i)^j * nchoosek(M,j);
    end
    for j=m:1:K
        prob_k(i,j) = prob_0(i) * rho(i)^j* nchoosek(M,j) * ( factorial(j) / factorial(m)) * m^(m-j);
    end
end

prob=zeros(9,7); %P0 ~ P6 확률 값을 모두 담은 prob 행렬
for i=1:9
    for j=1:7
        if j==1
            prob(i,j)=prob_0(i);
        else
            prob(i,j)=prob_k(i,j-1);
        end
    end
end

customer_number = zeros(1,9); %평균 Customer 수 저장
for i=1:1:9
    for j=1:1:K
        range = j * prob_k(i,j);
        customer_number(1,i) = customer_number(1,i) + range;
    end
end

%C 코드 Simulation으로 나온 결과 값, 평균 Customer 수
sim_customer_number = [1.724528 3.069216 3.964112 4.518749 4.870204 5.102230 5.259021 5.372889 5.457665 ];



%C 코드 Simulation으로 나온 결과 값, utilization factor가 0.1, 0.3, 0.5, 0.7, 0,9 일때 확률 값 
sim_prob=[0.171720 0.306346 0.272746 0.157138 0.065061 0.020906 0.006084
    0.009339 0.047723 0.119604 0.195375 0.232700 0.211301 0.183959
    0.001156 0.009006 0.036091 0.095820 0.185107 0.275790 0.397030
    0.000251 0.002472 0.013591 0.048778 0.129225 0.267965 0.537718
    0.000068 0.000887 0.006081 0.027709 0.092935 0.244168 0.628152];


%%%% average customer의 수 그래프 출력 %%%%
figure(1)
plot_i = 0.1:0.1:0.9;
plot(plot_i,customer_number,'-b')
hold on
plot(plot_i,sim_customer_number,'b*')
grid on
legend('Analysis', 'Simulation');
xlabel('Utilization Factor');
ylabel('Number of customers')


%%%% probability of the number of customer 그래프 출력 %%%%
figure(2)
k=0:1:6;

plot(k, prob(1,:), 'r-')
hold on
plot(k, sim_prob(1,:), 'r*');
hold on
grid on
plot(k, prob(3,:), 'b-')
hold on
plot(k, sim_prob(2,:), 'bo');
hold on
plot(k, prob(5,:), 'k-')
hold on
plot(k, sim_prob(3,:), 'kd');
hold on
plot(k, prob(7,:), 'm-')
hold on
plot(k, sim_prob(4,:), 'ms');
hold on
plot(k, prob(9,:), 'g-')
hold on
plot(k, sim_prob(5,:), 'g^');
hold on
grid on
legend('\rho=0.1 Analysis', '\rho=0.1 Simulation', '\rho=0.3 Analysis', '\rho=0.3 Simulation',...
    '\rho=0.5 Analysis', '\rho=0.5 Simulation', '\rho=0.7 Analysis', '\rho=0.7 Simulation',...
    '\rho=0.9 Analysis', '\rho=0.9 Simulation');
xlabel('Number of "k"');
ylabel('Probability')
