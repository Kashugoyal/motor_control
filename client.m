function client(port)
%   provides a menu for accessing PIC32 motor control functions
%

%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:client('/dev/
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;

% menu loop
while ~has_quit
    fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf('a: Read current sensor (counts) b: Read current senson (mA) \nc: Read encoder (counts)        d: Read encoder (deg)\ne: reset encoder                f: Set PWM (-100 to 100) \ng: Set Current Gains            h: Get Current Gains\np: Unpower the motor            r: Get mode\nq: Quit\n');
    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
   
    % take the appropriate action
    switch selection
        case 'd'                         % example operation
            motor_degree = fscanf(mySerial,'%d');
            fprintf('The motor angle is %d degrees.\n', motor_degree)
        case 'c'
            motor_counts = fscanf(mySerial,'%d');
            fprintf('The motor angle is %d counts.\n', motor_counts)
        case 'e'
            motor_reset = fscanf(mySerial,'%d');
            fprintf('The motor angle is %d counts.It is reset\n', motor_reset)
        case 'q'
            has_quit = true;             % exit client
        case 'a'
            adc_counts = fscanf(mySerial,'%d');
            fprintf('The ADC counts is %d\n', adc_counts)
        case 'b'
            adc_ma = fscanf(mySerial,'%d');
            fprintf('The current is %d mA\n', adc_ma)
        case 'r'
            get_mode = fscanf(mySerial,'%d');
            fprintf('%d\n', get_mode) 
        case 'p' 
            motor_unpower = fscanf(mySerial,'%d');
            fprintf('PWM has been set to %d\n', motor_unpower)        
        case 'f'
            value = input('\nENTER COMMAND: ', 's');
            fprintf(mySerial,'%s\n',value);
            pwm = fscanf(mySerial,'%f');
            fprintf('PWM has been set to %f\n', pwm)
        case 'g'
            kpki = input('Set in the form [Kp Ki]: ');
            fprintf(mySerial,'%f %f\n',kpki);
        case 'h'
             a = fscanf(mySerial,'%f %f');
             fprintf(['Kp: ',num2str(a(1)),' Ki: ',num2str(a(2)),' \n' ]);
        case 'i'
            kpmkim = input('Set in the form [Kpm Kdm Kim]: ');
            fprintf(mySerial,'%f %f %f\n',kpmkim);
        case 'k'
             fprintf('ITEST Mode\n');
             data = read_plot_matrix(mySerial);
             plot(data);
        case 'l'
            motorangle = input('\nENTER COMMAND: ', 's');
            fprintf(mySerial,'%s\n',motorangle);
        case 'j'
             b = fscanf(mySerial,'%f %f %f');
             fprintf(['Motor Kp: ',num2str(b(1)),'\tMotor Kd: ',num2str(b(2)),'Motor Ki: ',num2str(b(3)),' \n' ]);
        case 'm'
            reflist = input('Enter the Values as [time1 angle1 ; time2 angle2 ; ...]');
            ref = genRef(reflist, 'step');
           fprintf('Length is : %d\n', length(ref));
           fprintf(mySerial,'%d\n',length(ref));           
           for i = 1:length(ref)
              fprintf(mySerial,'%f\n',ref(i));  
           end    
        case 'n'
            reflist = input('Enter the Values as [time1 angle1 ; time2 angle2 ; ...]');
            ref = genRef(reflist, 'cubic');
           fprintf('Length is : %d\n', length(ref));
           fprintf(mySerial,'%d\n',length(ref));           
           for i = 1:length(ref)
              fprintf(mySerial,'%f\n',ref(i));  
           end    
        case 'z'
             %for i = 1:length(ref)
            b = fscanf(mySerial,'%f');
            fprintf('%f\n',b);
             %end   
              
        case 'o'
            data = read_plot_matrix(mySerial);
            plot(data);
      otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end
% 60 200 0.02
% 27 500 0.01