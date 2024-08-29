% Define input image
input_fname = 'mean.nii.gz';

% Define Frangi filter parameters
filtering_options = struct();
filtering_options.Thickness = 1;
filtering_options.ObjectPolarity = 'dark';

% Define segmentation parameters
segmentation_options.Threshold = 1e-5;

% Load input image and information
info = niftiinfo(input_fname);
input_image = niftiread(input_fname);

% Create brain tissue segmentation map
samseg_command_pattern = 'run_samseg --input %s --output %s --threads 10';
mri_convert_command_pattern = 'mri_vol2vol --mov tmp/seg.mgz --targ %s --o tmp/seg.nii.gz --regheader --nearest';
samseg_command = sprintf(samseg_command_pattern, input_fname, 'tmp/');
mri_convert_command = sprintf(mri_convert_command_pattern, input_fname);
system(samseg_command);
system(mri_convert_command);

% Create regions of interest
ROIs = [2, 41, 51, 52, 12, 13, 10, 49, 11, 50, 26, 58, 77, 80];
tissue_seg = niftiread('tmp/seg.nii.gz');
seg_mask = int16(ismember(tissue_seg, ROIs));

info.Datatype = class(seg_mask);
niftiwrite(seg_mask, 'ROI', info, 'Compressed', 1);

% Filter using the Frangi filter
frangi_filtered_image = wrapper_frangi_filtering(input_image, filtering_options);

% Exclude regions outside ROI
frangi_filtered_image(seg_mask == 0) = 0;

info.Datatype = class(frangi_filtered_image);
niftiwrite(frangi_filtered_image, 'filtered', info, 'Compressed', 1);

% Apply global thresholding
output_image = int16(frangi_filtered_image > segmentation_options.Threshold);

% Save segmentation
info.Datatype = class(output_image);
niftiwrite(output_image, 'segmentation', info, 'Compressed', 1);