case 'K': {
                ++m_DataIt;
                if (*m_DataIt == 'a') // Ambient color
                {
                    ++m_DataIt;
                    getColorRGBA(&m_pModel->m_pCurrentMaterial->ambient);
                } else if (*m_DataIt == 'd') {
                    // Diffuse color
                    ++m_DataIt;
                    getColorRGBA(&m_pModel->m_pCurrentMaterial->diffuse);
                } else if (*m_DataIt == 's') {
                    ++m_DataIt;
                    getColorRGBA(&m_pModel->m_pCurrentMaterial->specular);
                } else if (*m_DataIt == 'e') {
                    ++m_DataIt;
                    getColorRGBA(&m_pModel->m_pCurrentMaterial->emissive);
                }
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;
            case 'T': {
                ++m_DataIt;
                // Material transmission color
                if (*m_DataIt == 'f')  {
                    ++m_DataIt;
                    getColorRGBA(&m_pModel->m_pCurrentMaterial->transparent);
                } else if (*m_DataIt == 'r')  {
                    // Material transmission alpha value
                    ++m_DataIt;
                    ai_real d;
                    getFloatValue(d);
                    m_pModel->m_pCurrentMaterial->alpha = static_cast<ai_real>(1.0) - d;
                }
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;
            case 'd': {
                if (*(m_DataIt + 1) == 'i' && *(m_DataIt + 2) == 's' && *(m_DataIt + 3) == 'p') {
                    // A displacement map
                    getTexture();
                } else {
                    // Alpha value
                    ++m_DataIt;
                    getFloatValue(m_pModel->m_pCurrentMaterial->alpha);
                    m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
                }
            } break;

            case 'N':
            case 'n': {
                ++m_DataIt;
                switch (*m_DataIt) {
                    case 's': // Specular exponent
                        ++m_DataIt;
                        getFloatValue(m_pModel->m_pCurrentMaterial->shineness);
                        break;
                    case 'i': // Index Of refraction
                        ++m_DataIt;
                        getFloatValue(m_pModel->m_pCurrentMaterial->ior);
                        break;
                    case 'e': // New material
                        createMaterial();
                        break;
                    case 'o': // Norm texture
                        --m_DataIt;
                        getTexture();
                        break;
                }
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;

            case 'P':
                {
                    ++m_DataIt;
                    switch(*m_DataIt)
                    {
                    case 'r':
                        ++m_DataIt;
                        getFloatValue(m_pModel->m_pCurrentMaterial->roughness);
                        break;
                    case 'm':
                        ++m_DataIt;
                        getFloatValue(m_pModel->m_pCurrentMaterial->metallic);
                        break;
                    case 's':
                        ++m_DataIt;
                        getColorRGBA(m_pModel->m_pCurrentMaterial->sheen);
                        break;
                    case 'c':
                        ++m_DataIt;
                        if (*m_DataIt == 'r') {
                            ++m_DataIt;
                            getFloatValue(m_pModel->m_pCurrentMaterial->clearcoat_roughness);
                        } else {
                            getFloatValue(m_pModel->m_pCurrentMaterial->clearcoat_thickness);
                        }
                        break;
                    }
                    m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
                }
                break;

            case 'm': // Texture
            case 'b': // quick'n'dirty - for 'bump' sections
            case 'r': // quick'n'dirty - for 'refl' sections
            {
                getTexture();
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;

            case 'i': // Illumination model
            {
                m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
                getIlluminationModel(m_pModel->m_pCurrentMaterial->illumination_model);
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;

            case 'a': // Anisotropy
            {
                ++m_DataIt;
                getFloatValue(m_pModel->m_pCurrentMaterial->anisotropy);
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;