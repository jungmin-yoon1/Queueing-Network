m = 10;  %서버 수 
K = 15;   %큐의 크기
M = 20;  %전체 시스템 Customer 수

range = 1:9;
rho = range./10; %utilization factor : 0.1 ~0.9

syms k
Series1 = symsum(rho.^k.*nchoosek(M,k),k,0,m-1);  %symsum:  (1/10)^k*MCk ,k가 0~m-1 까지 일때 급수 합
Series2 = symsum(rho.^k.*nchoosek(M,k).*factorial(k)./factorial(m)*(m^(m-k)),k,m,K);
prob_0 = 1./(Series1+Series2); %P0 확률

prob_k = zeros(9,6); %P1 ~ P6 확률 저장
for i=1:1:9
    for j=1:1:m-1
        prob_k(i,j) = prob_0(i) * rho(i)^j * nchoosek(M,j);
    end
    for j=m:1:K
        prob_k(i,j) = prob_0(i) * rho(i)^j* nchoosek(M,j) * ( factorial(j) / factorial(m)) * m^(m-j);
    end
end

prob=zeros(9,7);  %P0 ~ P6 확률 값을 모두 담은 prob 행렬
for i=1:9
    for j=1:7
        if j==1
            prob(i,j)=prob_0(i);
        else
            prob(i,j)=prob_k(i,j-1);
        end
    end
end

customer_number = zeros(1,9);  %평균 Customer 수 저장
for i=1:1:9
    for j=1:1:K
        range = j * prob_k(i,j);
        customer_number(1,i) = customer_number(1,i) + range;
    end
end

%C 코드 Simulation으로 나온 결과 값, 평균 Customer 수
sim_customer_number = [1.722672 3.160856 4.370211 5.418007 6.344083 7.174093 7.938805 8.632014 9.272132 ];


%%%% average customer의 수 그래프 출력 %%%%

plot_i = 0.1:0.1:0.9;
plot(plot_i,customer_number,'-b')
hold on
plot(plot_i,sim_customer_number,'b*')
grid on
legend('Analysis', 'Simulation');
xlabel('Utilization Factor');
ylabel('Number of customers')

