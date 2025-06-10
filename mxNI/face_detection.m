%% Project - Time-of-Flight Camera
clear; clc; close all;

%% Section 0 - Create MATLAB wrapper for ToF Camera
% Compile cpp file from parent folder to create Matlab executable wrapper
% mxNI.mexw64

run('..\compile_cpp_files.m');


%% Section 1 - Initialization
% Set config file parameters - Make sure ToF camera is closed/disconneted
% function modify_config(lens_mode),
% lens_mode = 0 for near mode, 1 for normal
% Make sure the file C:\Program Files\LIPSToF\ModuleConfig.json has
% permissions set so this user can write to it.
modify_config(0);

% Initialize ToF camera
mxNI(0);
% mxNI(5); % Turning on depth + image registration

% Initialize variables to store experimental data
ranges_truth_cm = 30:15:90;

mean_depth_ranging = zeros(1, length(ranges_truth_cm));
mean_intensity_ranging =  zeros(1, length(ranges_truth_cm));
std_depth_ranging = zeros(1, length(ranges_truth_cm));
std_intensity_ranging =  zeros(1, length(ranges_truth_cm));

% Setting depth mode 0, color mode 0 and IR mode 0
mxNI(13,0); % mxNI command to initialize depth capture
mxNI(14,0); % mxNI command to initialize RGB/color capture
mxNI(15,0); % mxNI command to initialize IR capture

%% Section 2 - Data capture
clc
close all;

% MANUALLY ADJUST current_range variable 
current_range = 90; 

% If the estimate of Pixel_Y, Pixel_X is off-target, manually override it by uncommenting 
% the two lines below and entering values of Pixel_Y and Pixel_X by using datatips in the 
% saved fig file to find target center in the IR image 
% Pixel_X = 320;
% Pixel_Y = 230;

% Capture 1 frame to get fram+++++e information
[depth0_fr, ~] = mxNI(2);  % mxNI command to capture depth image
depth0_fr = fliplr(depth0_fr);
[color0_fr, ~] = mxNI(3);  % mxNI command to capture RGB image
color0_fr = fliplr(color0_fr);
[ir0_fr, ~] = mxNI(6);     % mxNI command to capture IR image
ir0_fr = fliplr(ir0_fr);

% Capturing 100 (N) successive frames to find statistics for estimated
% depth and received echo intensity across captures
N = 100; 
depth0_fr_N = zeros(size(depth0_fr,1),size(depth0_fr,2),N);
ir0_fr_N = zeros(size(ir0_fr,1),size(ir0_fr,2),N);

for i = 1:N 
    [depth0_fr,~] = mxNI(2);
    depth0_fr = fliplr(depth0_fr);
    [ir0_fr,~] = mxNI(6);
    ir0_fr = fliplr(ir0_fr);
    depth0_fr_N(:,:,i) = cast(depth0_fr,'double')./10; % converting to cm
    ir0_fr_N(:,:,i) = cast(ir0_fr,'double');
end

depth_mean = mean(depth0_fr_N,3); 
ir_mean = mean(ir0_fr_N,3);

depth_max = prctile(depth_mean(:), 80);
depth_min = prctile(depth_mean(:), 20);

ir_max = prctile(ir_mean(:), 80);
ir_min = prctile(ir_mean(:), 20);

%% Section 3- face bounding box
faceDetector = vision.CascadeObjectDetector();
image_cap      = color0_fr;
bbox            = step(faceDetector, image_cap);

% Draw the returned bounding box around the detected face.
image_cap = insertShape(image_cap, 'Rectangle', bbox);
figure; imshow(image_cap); title('Detected face');