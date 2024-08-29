function output_image = wrapper_frangi_filtering(input_image, provided_filtering_options)
    filtering_options_final = provided_filtering_options;

    if ~isfield(filtering_options_final, 'Thickness')
        error('Frangi filtering requires a thickness parameter');
    end

    if ~isfield(filtering_options_final, 'StructureSensitivity')
       filtering_options_final.StructureSensitivity = 0.01*diff(getrangefromclass(input_image)); 
    end

    if ~isfield(filtering_options_final, 'ObjectPolarity')
       filtering_options_final.object_polarity = 'bright'; 
    end

    output_image = fibermetric(...
        input_image, ...
        provided_filtering_options.Thickness, ...
        'StructureSensitivity', filtering_options_final.StructureSensitivity, ...
        'ObjectPolarity', filtering_options_final.ObjectPolarity);
end