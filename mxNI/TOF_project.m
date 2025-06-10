%% EE 219- Time-of-Flight Project
% Group members: Wen-Chieh Chao, Caelia Thomas, Ronan Locker, Xiangren Chen
clear; clc; close all;
mkdir data;
mkdir plots;
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
lens_mode = 0;
modify_config(lens_mode);
% Initialize ToF camera
mxNI(0);
mxNI(5); % Turning on depth + image registration
% Setting depth mode 0, color mode 0 and IR mode 0
mxNI(13,0); % mxNI command to initialize depth capture
mxNI(14,0); % mxNI command to initialize RGB/color capture
mxNI(15,0); % mxNI command to initialize IR capture
%% Section 2 - Data Capture
% Recommended data file name: person_realorfake
% eg. william_real
scene_name = 'Reg_TA';
[depth, ~] = mxNI(2);  % mxNI command to capture depth image
depth = fliplr(depth);
[color, ~] = mxNI(3);  % mxNI command to capture RGB image
color = fliplr(color);
[ir, ~] = mxNI(6);     % mxNI command to capture IR image
ir = fliplr(ir);
% save to mat file
save(sprintf('data/%s', scene_name), 'depth', 'color', 'ir');
plot_tof_data(scene_name);
%% Section 3 - Data Analysis
scene_name = 'Prof2_fake_normalmode_far_angledSIDE';
load(sprintf('data/%s', scene_name));
plot_tof_data(scene_name);
%%% YOUR CODE HERE %%%
[SS, image_cap, cropped_depth] = SS_res_generator(color, depth, 0);
figure; imshow(image_cap); title('Detected face');
figure; imshow(cropped_depth);clim([300,800])
mask = cropped_depth > 0;
minD = min(cropped_depth(mask));
rangeD = 100;
[H, W] = size(cropped_depth);
three_dim = false(H, W, rangeD);
for i = 1:H
 for j = 1:W
     d = cropped_depth(i,j);
     k = round(d - minD + 1);
     if k >= 1 && k <= rangeD && d > 0
         three_dim(i,j,k) = true;
     end
 end
end
% Plot voxels
[i_idx, j_idx, k_idx] = ind2sub(size(three_dim), find(three_dim));
figure;
scatter3(j_idx, i_idx, k_idx, 6, k_idx, 'filled');
axis equal tight;
view(3);
title('3D Voxel Plot');
colorbar;
%%% END OF YOUR CODE %%%
%% Section 4 - Live Demo
while true
 [depth, ~] = mxNI(2);  % mxNI command to capture depth image
 depth = double(fliplr(depth));
 [color, ~] = mxNI(3);  % mxNI command to capture RGB image
 color = fliplr(color);
 [ir, ~] = mxNI(6);     % mxNI command to capture IR image
 ir = double(fliplr(ir));
 %%% YOUR CODE HERE %%%
 %%% END OF YOUR CODE %%%
end
%% Section 4.5 - Live Demo Test
clc
% Capture 1 frame to get frame information
[depth0_fr, ~] = mxNI(2);  % mxNI command to capture depth image
depth0_fr = cast(fliplr(depth0_fr),'double');
[color0_fr, ~] = mxNI(3);  % mxNI command to capture RGB image
color0_fr = fliplr(color0_fr);
[ir0_fr, ~] = mxNI(6);     % mxNI command to capture IR image
ir0_fr = fliplr(ir0_fr);
depth_max = 1500;
depth_min = 0;
ir_max = prctile(ir0_fr(:), 90);
ir_min = prctile(ir0_fr(:), 10);
figure;
subplot(1,3,1)
imagesc(depth0_fr); caxis([depth_min depth_max]);
axis image
xlabel('Pixels (x)');
ylabel('Pixels (y)');
title('Depth Image')
subplot(1,3,2)
imagesc(ir0_fr); caxis([ir_min ir_max]);
axis image
xlabel('Pixels (x)');
ylabel('Pixels (y)');
title('IR Image')
subplot(1,3,3)
imagesc(color0_fr);
axis image
xlabel('Pixels (x)');
ylabel('Pixels (y)');
title('Color Image')
i=1;
while(true)
 [depth0_fr,~] = mxNI(2);
 depth0_fr = cast(fliplr(depth0_fr),'double');
 [ir0_fr,~] = mxNI(6);
 ir0_fr = fliplr(ir0_fr);
 [color0_fr, ~] = mxNI(3);
 color0_fr = fliplr(color0_fr);
  if mod(i,10)==0
     depth_max = 1500;
     depth_min = 0;
  
     ir_max = prctile(ir0_fr(:), 90);
     ir_min = prctile(ir0_fr(:), 20);
 end
  % Classification
 [SS, image_cap, cropped_depth] = SS_res_generator(color0_fr, depth0_fr, lens_mode);
  subplot(1,2,1)
 imagesc(depth0_fr); caxis([depth_min depth_max]);
 colormap(jet)
 axis image
 xlabel('Pixels (x)');
 ylabel('Pixels (y)');
 title('Depth Image')
 subplot(1,2,2)
 imagesc(image_cap);
 axis image
 title(SS);
 getframe;
 i=i+1;
end
%% Section 5 - Data Analysis
clc;
folder = 'data';
files = dir(fullfile(folder, '*.mat'));
%figure;
for k = 1:length(files)
 scene_name = files(k).name;
 load(sprintf('data/%s', scene_name));
 [SS, image_cap, cropped_depth] = SS_res_generator(color, depth, 0);
 if SS > 1000
     disp("File Name: " + scene_name + "   REAL! , SS_res: " + SS);
 else
     disp("File Name: " + scene_name + "   FAKE! , SS_res: " + SS);
 end
end
%% Section 6 - Closing the device
mxNI(1);
%% Function definitions
function [] = plot_tof_data(scene_name)
 load(sprintf('data/%s', scene_name), 'depth', 'color', 'ir');
 figure(1);
 set(gcf, 'Color', 'w');
 set(gcf, 'Position', [0 0 1500 500]);
  subplot(131);
 imshow(color);
 xlabel('Pixels (x)');
 ylabel('Pixels (y)');
 title('RGB Image');
 axis image;
  subplot(132);
 imagesc(depth);
 xlabel('Pixels (x)');
 ylabel('Pixels (y)');
 title('Depth Image');
 axis image;
 colorbar;
  subplot(133);
 imagesc(ir);
 title('IR Image');
 axis image;
 colorbar;
  sgtitle(sprintf('Unprocessed data for %s', scene_name), 'Interpreter', 'none');
 saveas(gcf, sprintf('plots/%s_data.png', scene_name));
end
% feel free to define your own functions below!
%%% YOUR CODE HERE %%%
function [SS, image_cap, cropped_depth] = SS_res_generator(color, depth, lens_mode)
 % face detection, KLT
 faceDetector = vision.CascadeObjectDetector();
 image_cap      = color;
 bbox            = step(faceDetector, image_cap);
 cropped_depth = NaN;
 if isequal(size(bbox), [1, 4]) % detected!
     % = insertShape(image_cap, 'Rectangle', bbox);
	% Crop hair!
     bbox = [bbox(1), bbox(2) + bbox(4)*0.2, bbox(3), bbox(4)*0.8];
	% Median filter to remove the outlier
     depth_filtered = medfilt2(depth, [9 9]);
   	% Crop the face
     cropped_depth = imcrop(depth_filtered, bbox);
% Swap zeros for NaN
     cropped_depth = double(cropped_depth);
     cropped_depth(cropped_depth == 0) = NaN;
   	
	% Average vertical depth profile
     profile_vertical = mean(cropped_depth, 2, 'omitnan');
     y_vertical = profile_vertical';
     x_vertical = (1:length(profile_vertical));
  
	% Average horizontal depth profile
     profile_horizontal = mean(cropped_depth, 1,'omitnan');
     y_horizontal = profile_horizontal;
     x_horizontal = (1:length(profile_horizontal));
  
	% filter out NaN
     idx_horizontal = isnan(y_horizontal);
     idx_vertical = isnan(y_vertical);
  
	% linear regression on Average vertical depth profile
     p_vertical = polyfit(x_vertical(~idx_vertical),y_vertical(~idx_vertical),1);
     yfit_vertical = polyval(p_vertical,x_vertical);
  
	% linear regression on Average horizontal depth profile
     p_horizontal = polyfit(x_horizontal(~idx_horizontal),y_horizontal(~idx_horizontal),1);
     yfit_horizontal = polyval(p_horizontal,x_horizontal);
  
	%
     SS_res_vertical = sum((y_vertical(~idx_vertical) - yfit_vertical(~idx_vertical)).^2);
     SS_res_horizontal = sum((y_horizontal(~idx_horizontal) - yfit_horizontal(~idx_horizontal)).^2);
     SS = min(SS_res_vertical, SS_res_horizontal);
     %if SS > 500
     if isequal(lens_mode, 0)
       threshold = 500;
     else
       threshold = 5000;
     end
     if SS > threshold
        image_cap = insertShape(image_cap, 'Rectangle', bbox, 'Color', 'green', 'LineWidth', 5);
        image_cap = insertText(image_cap, [230, 0], 'REAL', 'FontSize', 64, 'BoxColor', 'green', 'TextColor', 'white');
     else
        image_cap = insertShape(image_cap, 'Rectangle', bbox, 'Color', 'red', 'LineWidth', 5);
        image_cap = insertText(image_cap, [230, 0], 'FAKE', 'FontSize', 64, 'BoxColor', 'red', 'TextColor', 'white');
     end
     %disp("File Name: " + scene_name + ", SS_res: " + SS_res_vertical + " and " + SS_res_horizontal + ", SS_total: " + SS_tot_vertical + " and " + SS_tot_horizontal + ", Rsquare: " + Rsq_vertical + " and " + Rsq_horizontal);
 else
     SS = 0;
 end
end
%%% END OF YOUR CODE %%%

