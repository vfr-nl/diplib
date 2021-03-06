% Real-valued scalar
x=newim(40,50,20);
x(:)=0;
x(:,1)=4; %error
x(:,0,0)=4;
x(:,0,0)=1:40;
x(:,:,1)=1:(40*50); % error
x(:,:,1)=newim(40,50);

% Complex-valued scalar
x=newim(40,50,20,'scomplex');
x(:)=0;
x(:,1)=4; %error
x(:,0,0)=4;
x(:,0,0)=1:40;
x(:,:,1)=1:(40*50); % error
x(:,:,1)=newim(40,50);
x(:)=5+1i;
x(:,0,0)=4i;
x(:,0,0)=(1:40)+1i;
x(:,:,1)=newim(40,50,'dcomplex');

% Real-valued tensor
x=newtensorim(3,[40,50,20]);
x(:)=0;
x(:,1)=4; %error
x(:,0,0)=4;
x(:,0,0)=1:40;
x(:,:,1)=1:(40*50); % error
x(:,:,1)=newim(40,50);

x{1}(:)=0;
x{1}(:,0,0)=4;
x{1}(:,0,0)=1:40;
x{1}(:,:,1)=1:(40*50); % error
x{1}(:,:,1)=newim(40,50);

x{:}(:)=0;
x{:}(:,1)=4; %error
x{:}(:,0,0)=4;
x{:}(:,0,0)=1:40;
x{:}(:,:,1)=1:(40*50); % error
x{:}(:,:,1)=newim(40,50);

x{:}(:,:,1)=newtensorim(3,[40,50]);
x{1}(:,:,1)=newtensorim(3,[40,50]); % error

% Complex-valued tensor
x=newtensorim(3,[40,50,20],'scomplex');
x(:)=0;
x(:,0,0)=4;
x(:,0,0)=1:40;
x(:,:,1)=newim(40,50);

x{1}(:)=0;
x{1}(:,0,0)=4;
x{1}(:,0,0)=1:40;
x{1}(:,:,1)=newim(40,50);

x{:}(:)=0;
x{:}(:,0,0)=4;
x{:}(:,0,0)=1:40;
x{:}(:,:,1)=newim(40,50);

x{:}(:,:,1)=newtensorim(3,[40,50]);

x(:)=5+1i;
x(:,0,0)=4i;
x(:,0,0)=(1:40)+1i;
x(:,:,1)=newim(40,50,'dcomplex');

x{1}(:)=5+1i;
x{1}(:,0,0)=4i;
x{1}(:,0,0)=(1:40)+1i;
x{1}(:,:,1)=newim(40,50,'dcomplex');

x{:}(:)=5+1i;
x{:}(:,0,0)=4i;
x{:}(:,0,0)=(1:40)+1i;
x{:}(:,:,1)=newim(40,50,'dcomplex');

x{:}(:,:,1)=newtensorim(3,[40,50],'dcomplex');
