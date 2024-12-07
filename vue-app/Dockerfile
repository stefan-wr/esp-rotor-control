FROM node:lts-alpine

RUN mkdir -p /rotor-controller/include
WORKDIR /rotor-controller/vue-app

COPY package.json package-lock.json ./
RUN npm install --legacy-peer-deps

COPY . .

EXPOSE 5173

#CMD ["npm", "run", "dev"]