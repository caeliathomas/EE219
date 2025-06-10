figure;
subplot(1,3,1)
imagesc(depth_mean); caxis([0 100]);
colormap(jet)
axis image
xlabel('Pixels (x)');
ylabel('Pixels (y)');
title('Depth Image')
subplot(1,3,2)
imagesc(ir_mean); caxis([ir_min ir_max]);
colormap(jet)
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